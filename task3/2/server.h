#pragma once
#include <queue>
#include <unordered_map>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <stdexcept>

template<typename T>
class Server {
public:
    using task_func = std::function<T()>;
    
    Server() = default;
    ~Server() { stop(); }
    
    // Интерфейс
    void start();
    void stop();
    size_t add_task(task_func task);      // Возвращает ID задачи
    T request_result(size_t id);          // Возвращает результат (блокирующий)
    
    // Для бенчмаркинга: статистика
    size_t get_processed_count() const { return processed_count_.load(); }
    
private:
    void worker();  // Поток-обработчик
    
    // Очередь задач: ID + функция
    std::queue<std::pair<size_t, task_func>> task_queue_;
    
    // Промисы для установки результатов
    std::unordered_map<size_t, std::promise<T>> promises_;
    
    // Shared_future для получения результатов клиентами
    std::unordered_map<size_t, std::shared_future<T>> futures_;
    
    // Синхронизация
    std::mutex queue_mutex_;
    std::mutex results_mutex_;
    std::condition_variable cv_;
    
    std::thread worker_thread_;
    std::atomic<bool> running_{false};
    std::atomic<size_t> next_id_{0};
    std::atomic<size_t> processed_count_{0};
};

// === Реализация методов ===

template<typename T>
void Server<T>::start() {
    if (running_.exchange(true)) return;  // Уже запущен
    worker_thread_ = std::thread(&Server::worker, this);
}

template<typename T>
void Server<T>::stop() {
    if (!running_.exchange(false)) return;  // Уже остановлен
    cv_.notify_all();  // Разбудить рабочий поток
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    // Очистка после остановки
    std::lock_guard<std::mutex> lock(results_mutex_);
    promises_.clear();
    futures_.clear();
}

template<typename T>
size_t Server<T>::add_task(task_func task) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    size_t id = next_id_++;
    
    // Создаём promise/future пару
    std::promise<T> promise;
    std::shared_future<T> future = promise.get_future().share();
    
    // Сохраняем для последующего использования
    promises_.emplace(id, std::move(promise));
    futures_.emplace(id, future);
    
    // Добавляем задачу в очередь
    task_queue_.emplace(id, std::move(task));
    
    // Уведомляем рабочий поток
    cv_.notify_one();
    return id;
}

template<typename T>
T Server<T>::request_result(size_t id) {
    std::shared_future<T> future;
    {
        std::lock_guard<std::mutex> lock(results_mutex_);
        auto it = futures_.find(id);
        if (it == futures_.end()) {
            throw std::runtime_error("Task ID " + std::to_string(id) + " not found");
        }
        future = it->second;  // shared_future копируемый
    }
    // Блокирующее ожидание результата
    return future.get();
}

template<typename T>
void Server<T>::worker() {
    while (true) {
        std::pair<size_t, task_func> task_item;
        
        // Извлечение задачи из очереди
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { 
                return !task_queue_.empty() || !running_; 
            });
            
            // Завершение при остановке и пустой очереди
            if (task_queue_.empty() && !running_) break;
            if (task_queue_.empty()) continue;
            
            task_item = std::move(task_queue_.front());
            task_queue_.pop();
        }
        
        size_t id = task_item.first;
        
        // Выполнение задачи с обработкой исключений
        try {
            T result = task_item.second();
            
            // Установка результата в promise
            {
                std::lock_guard<std::mutex> lock(results_mutex_);
                auto it = promises_.find(id);
                if (it != promises_.end()) {
                    it->second.set_value(std::move(result));
                    // future оставляем для request_result
                }
            }
            processed_count_++;
            
        } catch (...) {
            // Проброс исключения через future
            std::lock_guard<std::mutex> lock(results_mutex_);
            auto it = promises_.find(id);
            if (it != promises_.end()) {
                it->second.set_exception(std::current_exception());
            }
        }
    }
}