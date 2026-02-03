# Task1
---

## Сборка

```bash
# Для типа float
cmake -S. -B build -DNUM_TYPE=Float
# Для типа double
cmake -S. -B build -DNUM_TYPE=Double

cmake --build build
./build/my_app
```
## Вывод
Float
```bash
Sum: 0.0943487
```
Double
```bash
Sum: 2.68431e-07 
```
