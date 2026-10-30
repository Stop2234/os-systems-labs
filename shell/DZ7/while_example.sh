#!/bin/bash
# Скрипт выводит числа от 1 до 5
count=1

while [ $count -le 5 ]; do
    echo "Текущее число: $count"
    count=$((count + 1))
done
