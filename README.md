# Тестовое задание
# Макро Групп
# Junior Программист С 

Условие задачи:
Дан файл с кодом (prob1.cpp), в котором описаны следующие структуры:
- image: Структура изображения: 
    + width, height – размеры изображения в пикселях; 
    + format – формат изображения:
        + GRAY серое изображение (на каждый пиксель 8 бит);
        + RGB/BGR цветное изображение (на каждый пиксель 24 бита). 
    + data – массив хранящий непосредственное изображение. Элементызанимают 8 бит, соответственно для цветных форматов информация опикселяхидет последовательно, на каждый пиксель три элемента для соответствующихцветов. 
  
- box: Структура, описывающая обнаруженные объекты: 
    + x1, y1 – координаты левой верхней точки рамки; 
    + x2, y2 – координаты правой нижней точки рамки; 
    + type – тип объекта (0-лицо, 1-пистолет, 2-маска). 
  
- frame: Структура, описывающая кадр: 
    + img – структура изображения кадра; 
    + boxes – массив структур обнаруженных объектов, которые отрисовываются. 
  
## В качестве тестового задания вам предлагается написать код для функций:
1. ``` bool rgb2bgr(image &img); ```
функция должна преобразовывать передаваемое в качестве аргумента изображение из формата RGB в формат BGR. Функция должна возвращатьtrueвслучае успешного преобразования, и false если преботразование выполнитьневозможно.

2. ``` void frame_clean(frame& f, float threshold); ```
В результате работы нейронной сети часто один объект детектируется несколькораз, это означает что кадр содержит несколько рамок, которые определяютодинитот же реальный объект. Для решения этой проблемы будем считать чтодвеструктуры box одного типа описывают один и тот же объект, если геометрическоепересечение их прямоугольников (IoU) больше либо равно пороговогозначенияthreshold. Считать, что рамки не отсортированны.

3. ``` frame union_frames(frame& f1, frame& f2, float threshold); ```
Данная функция должна обработать два кадра и в результате вернуть третийкадр,с тем же изображением, но с объединенными рамками двух кадров. Другимисловами массив рамок результирующего изображения должен содержатьпоодной рамке для каждого объекта, обнаруженных на двух кадрах. Так жебудемсчитать что две структуры box одного типа описывают один и тот же объект, еслигеометрическое пересечение их прямоугольников (IoU) больше либо равнопорогового значения threshold. Считать, что рамки не отсортированны.
