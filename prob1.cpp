#include <iostream>
#include <vector>
#include <stdint.h>

#define F_GRAY 0
#define F_RGB 1
#define F_BGR 2

#define DEBAG

struct image
{
    int width, height;
    int format; // 0=GRAY 1bytePix | 1=RGB 2=BGR 3bytePix
    uint8_t *data;
};

struct box
{
    int x1, y1;
    int x2, y2;
    int type; // 0=FACE   1=GUN   2=MASK
};
void printBox(box *);

struct frame
{
    image img;
    std::vector<box> boxes;
};

//  RGB -> BGR
bool rgb2bgr(image &img)
{

    if (img.width == 0 || img.height == 0)
    {
        return false;
    }
    if (img.format != F_RGB)
    {
        return false;
    }
    // Вообще бы наверное в тру кетч обернуть работу с памятью дабы программа не завершалась аварийно при отсутсвие прав
    // но в тз особо нету указаний как оброватывать этот случай (что делать с целостностью данных по мимо вернуть фолс)
    uint8_t tmp = 0;
    size_t size = img.width * img.height;
    for (size_t i = 0; i < size; ++i)
    {
        tmp = img.data[i * 3];                 // R
        img.data[i * 3] = img.data[i * 3 + 2]; // B -> 1
        img.data[i * 3 + 2] = tmp;             // R -> 3
    }
    img.format = F_BGR;
    return true;
}

float area(const box &a)
{
    // std::cout << " area " << std::abs(1.0 * (a.x2 - a.x1) * (a.y2 - a.y1));
    return std::abs(1.0 * (a.x2 - a.x1) * (a.y2 - a.y1));
}

box *boxMerge(box &a, box &b, float threshold = 1)
{
    if (a.type != b.type)
    {
        return 0;
    }
    if (a.x2 <= b.x1 || a.x1 >= b.x2)
    {
        // Если левый верхний Б дальше чем правый нижний А
        // Если левый верхний А дальше чем правый нижний Б
        return 0;
    }
    if (a.y1 <= b.y2 || a.y2 >= b.y1)
    {
        return 0;
    }
    if (threshold > 1)
    {
        threshold /= 100;
    }
    float rez = 0;
    box *c = new box;
    c->type = a.type;
    c->x1 = std::max(a.x1, b.x1); // left
    c->y1 = std::min(a.y1, b.y1); // top
    c->x2 = std::min(a.x2, b.x2); // right
    c->y2 = std::max(a.y2, b.y2); // bottom
    // Я бы хотел больше информации про оценку пересечений
    rez = std::max(area(*c) / area(a), area(*c) / area(b));
#ifdef DEBAG
    std::cout << "\nFind box " << area(*c) / area(a) << " <a | b> " << area(*c) / area(b) << " >> " << rez;
    printBox(c);
#endif
    if (rez >= threshold)
    {
        c->x1 = std::min(a.x1, b.x1); // left
        c->y1 = std::max(a.y1, b.y1); // top
        c->x2 = std::max(a.x2, b.x2); // right
        c->y2 = std::min(a.y2, b.y2); // bottom
        return c;
    }
    delete c;
    return 0;
}

bool sortType(box a, box b)
{
    return a.type > b.type;
}
//  функция очищает кадр, оставляя одну рамку для общих объектов
//  объект считается общим для двух box, если их IOU >= threshold
void frame_clean(frame &f, float threshold)
{
    // Ваш код
    // Надо оптимизацией особо не задумываюсь дабы это тестовое
    // поэтому в лоб просто перебор комбинаций, а вреале уже по приоитету память|быстодействие
    // Пытался выбить случай когда при объединение некскольких пар боксов
    // получается пересечение которое начинает удовлетворять требованию

    // std::sort(f.boxes.begin(), f.boxes.end(), sortType);
    // от начальной сортировки позже отказался

    size_t size = f.boxes.size();
    for (auto i = 0; i < size - 1; ++i)
    {
        for (auto j = i + 1; j < size;)
        {
            box *tmp = boxMerge(f.boxes[i], f.boxes[j], threshold);
            if (tmp != 0)
            {
                f.boxes.push_back(*tmp);
                f.boxes.erase(f.boxes.begin() + j);
                f.boxes.erase(f.boxes.begin() + i);
                size--;
                j = i + 1;
            }
            else
            {
                ++j;
            }
        }
    }
}

//  функция объединяет обнаруженные объекты из двух кадров в один
//  объект считается общим для двух box, если их IOU >= threshold
//  гарантируется, что f1.img == f2.img
frame union_frames(frame &f1, frame &f2, float threshold)
{
    // Ваш код
    // Нужно ли глубокое копирование или поверхностное тоже допустимо?
    frame f3; // Я бы предпочел возвращать указатель на структуру, но по тз возвращается объект

    // ==== Копиовение IMG ====
    f3.img.format = f1.img.format;
    f3.img.width = f1.img.width;
    f3.img.height = f1.img.height;
    size_t size = f3.img.width * f3.img.height * (f1.img.format == F_GRAY ? 1 : 3);
    f3.img.data = new uint8_t[size];
    for (auto i = 0; i < size; ++i)
    {
        f3.img.data[i] = f1.img.data[i];
    }
    // ==== Конец копиовение IMG ====

    // ==== Копироваине Boxes ====
    f3.boxes.reserve(f1.boxes.size() + f2.boxes.size());
    f3.boxes.clear();
    // f3.boxes = f1.boxes;
    f3.boxes.insert(f3.boxes.end(), f1.boxes.begin(), f1.boxes.end());
    f3.boxes.insert(f3.boxes.end(), f2.boxes.begin(), f2.boxes.end());
    frame_clean(f3, threshold);
    // Что то умное про супер перемещение с обработкой на лету.. а не строки выше
    // ==== Конец копироваине Boxes ====

    return f3;
}

image *createImg(size_t size = 1, int frt = 0)
{
    // image* point = (image*)malloc(sizeof(image));
    image *point = new image;

    point->height = size;
    point->width = 1;
    point->format = frt;
    if (frt != F_GRAY)
    {
        size = size * 3;
    }

    point->data = new uint8_t[size];

    for (size_t i = 0; i < size; i++)
    {
        point->data[i] = std::rand() % 0xFF;
    }
    return point;
};

void imgPrintData(image *img)
{
    std::cout << std::endl;
    printf("h %d | w %d | f %d | ", img->height, img->width, img->format);
    size_t size = img->width * img->height * (img->format == F_GRAY ? 1 : 3);
    for (size_t i = 0; i < size; i++)
    {
        printf("0x%02x ", img->data[i]);
    }
}

box *createBox(int t, int left, int top, int right, int bottom)
{

    box *c = new box;
    c->type = t;
    c->x1 = left;   // left
    c->x2 = right;  // right
    c->y1 = top;    // top
    c->y2 = bottom; // bottom
    if (left > right || top < bottom)
    {
        printBox(c);
        std::cout << "???Tebe norm???" << std::endl;
    }
    return c;
}

void printBox(box *c)
{
    if (c)
        std::cout << " | " << c->type << " | " << c->x1 << " " << c->y1 << " " << c->x2 << " " << c->y2 << "\n";
}

int main()
{
    // ==== Псевдо тестим преобразования изображения ====
    bool rez = 0;
    image *tmp1 = createImg();
    imgPrintData(tmp1);

    image *tmp2 = createImg(5);
    imgPrintData(tmp2);

    image *tmp3 = createImg(2, 1);
    imgPrintData(tmp3);
    rez = rgb2bgr(*tmp3);
    imgPrintData(tmp3);
    std::cout << rez;
    image *tmp4 = createImg(2, 2);
    imgPrintData(tmp4);
    rez = rgb2bgr(*tmp4);
    imgPrintData(tmp4);
    std::cout << rez;

    delete tmp1;
    tmp1 = nullptr;
    delete tmp2;
    tmp2 = nullptr;
    delete tmp3;
    tmp3 = nullptr;
    delete tmp4;
    tmp4 = nullptr;
    // ==== Конец тестов преобразования изображения ====

    std::cout << std::endl
              << std::endl;

    // ==== Псевдо тестим поиск пересечений и выдачу объединения ====
    box *tmp = 0;
    box *middle = createBox(1, 2, 6, 6, 2);

    tmp = boxMerge(*middle, *middle, 1);
    std::cout << "New box (1x1)";
    printBox(tmp); // 1x1
    delete tmp;

    box *inMiddle = createBox(1, 3, 5, 5, 3);
    tmp = boxMerge(*middle, *inMiddle, 1); // find 2 6 3 5
    std::cout << "New box (inMiddle)";
    printBox(tmp); // rez 2 6 6 2 (middle)
    delete tmp;

    box *leftTop = createBox(1, 1, 7, 3, 5);
    tmp = boxMerge(*middle, *leftTop, 0); // find 2 6 3 5
    std::cout << "New box (leftTop)";
    printBox(tmp); // rez 1 7 6 2
    delete tmp;

    box *topMiddle = createBox(1, 3, 7, 5, 1);
    tmp = boxMerge(*middle, *topMiddle, 0); // find 3 6 5 2
    std::cout << "New box (topMiddle)";
    printBox(tmp); // rez 2 7 6 1
    delete tmp;

    box *noMerge = createBox(1, 100, 100, 150, 10);
    tmp = boxMerge(*middle, *noMerge, 0);
    std::cout << "New box?";
    printBox(tmp);
    delete tmp;

    // Я знаю про существование умных указателей
    // но пока предпочитаю боль
    delete middle;
    middle = nullptr;
    delete inMiddle;
    inMiddle = nullptr;
    delete leftTop;
    leftTop = nullptr;
    delete topMiddle;
    topMiddle = nullptr;
    delete noMerge;
    noMerge = nullptr;

    // ==== Конец тестов пересечений ====

    // Это я пытался понять почему он сходит с ума на итераторах..
    // std::vector<int> v{1, 2, 4, 5, 6, 7, 8, 9, 1, 2, 10, 15, 3};
    // size_t size = v.size();
    // for (auto i = 0; i < size - 1; ++i)
    // {
    //     for (auto j = i + 1; j < size;)
    //     {
    //         std::cout << "this " << v[i] << " | " << v[j] << " | i:" << i << " | j:" << j << " //\n";
    //         if (v[i] + v[j] < 30)
    //         {
    //             std::cout << " del " << v[i] << " | " << v[j] << " | i:" << i << " | j:" << j << " //\n";
    //             v.push_back(v[i] + v[j]);
    //             v.erase(v.begin() + j);
    //             v.erase(v.begin() + i);
    //             size--;
    //             j = i + 1;
    //             std::cout << " new " << v[i] << " | " << v[j] << " | i:" << i << " | j:" << j << " //\n";
    //         }
    //         else
    //         {
    //             ++j;
    //         }
    //     }
    //     std::cout << "\n@@@\n";
    // }

    return 0;
}
