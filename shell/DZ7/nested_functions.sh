#!/bin/bash
#Функция внутри функции
func1() {
    echo "func1: запускаю func2"
    func2
}

func2() {
    echo "func2: функция успешно вызвана"
}

func1
