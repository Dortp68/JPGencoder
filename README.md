# JPGencoder
Транскодер для формата jpg. Задча - дожать jpeg без искажений.

Идея заключается в том, чтобы из сжатого jpg файла получить кожфиициенты дискретного косинусного преобразования и дожать их эффективнее, затем декодировать обратнов jpg без потерь. Для DC коэффициентов используется алгоритм сжатия PPMd, для AC производится кодирование длин серий. Сжимать можно изображения с любым quality factor.
Приложены рабочие .exe файлы, исходники находятся в соответсвующих exe файлах.
Тесты проводились на изображениях с качеством 30 и 80.
![Полученные результаты](https://github.com/Dortp68/JPGencoder/blob/main/results.png)
