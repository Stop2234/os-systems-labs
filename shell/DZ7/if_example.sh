#!/bin/bash
filename="testfile.txt"

if [ -f "$filename" ]; then
    echo "Файл $filename существует."
else
    echo "Файл $filename не найден."
fi
