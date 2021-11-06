#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define max(a, b) ((a) >=(b)) ? (a) : (b)
typedef struct uint1024_t {
    unsigned char arr[155]; //структура хранит в unsigned char числа в диапозоне от 0 до 255, тогда можно хранить там 2 цифры, максимум 309 цифр - 155 unsigned char
} uint1024_t;

void scanf_value(uint1024_t* x);
void printf_value(uint1024_t* x);
uint1024_t from_unit(unsigned x);
uint1024_t add_op(uint1024_t x, uint1024_t y);
uint1024_t subtr_op(uint1024_t x, uint1024_t y);
uint1024_t mult_op(uint1024_t x, uint1024_t y);


int main(int argc, char* argv[]) {
    printf("Please input uint1024_t number to be scanned from input:\n");
    uint1024_t* number1;
    number1 = calloc(1, 155);
    scanf_value(number1);
    // uint1024_t* number2;
    // number2 = calloc(1, 155);
    // scanf_value(number2);
    int x;
    printf("Please input uint1024_t number to be generated from unsigned int x:\n");
    scanf("%d", &x);
    uint1024_t* number2;
    number2 = calloc(1, 155);
    *number2 = from_unit(x);
    uint1024_t* res;
    res = calloc(1, 155);

    printf("=========================\nuint1024_t number scanned from input is:\n");
    printf_value(number1);
    printf("uint1024_t number generated from unsigned int x is:\n");
    printf_value(number2);
    *res = add_op(*number1, *number2);
    printf("The summ of uint1024_t scanned and uint1024_t generated from unsigned int x is:\n");
    printf_value(res);
    *res = subtr_op(*number1, *number2);
    printf("The residual of uint1024_t scanned and uint1024_t generated from unsigned int x is:\n");
    printf_value(res);
    *res = mult_op(*number1, *number2);
    printf("The product of uint1024_t scanned and uint1024_t generated from unsigned int x is:\n");
    printf_value(res);

    free(number1);
    free(number2);
    free(res);

    return 0;
}


uint1024_t from_unit(unsigned int x) { // генерация uint1024_t из unsigned int x

    uint1024_t *temp; // создается временная структура, куда запишется результат перевода x в untit1024_t
    temp = calloc(1, 155);
    int i = 0;

    while (x != 0) {

        temp -> arr[i] = x % 100;  // в ячейку структуры uint1024_t по индексу i записывается последняя или две последние цифры числа x в виде одно или двузначного числа
        x = x / 100;
        i++;

    }

    return *temp;

}


uint1024_t add_op(uint1024_t x, uint1024_t y) { // функция сложения двух чисел типа uint1024_t

    uint1024_t *summ;               //переменная хранящая результат
    summ = calloc(1, 155);
    unsigned char carry = 0;        // перенос при сложении  
    unsigned char plasces_summ = 0; // сумма двух "разрядов" чисел типа uint1024_t

    for (unsigned char i = 0; i < 155; i++) {       // с увеличением индекса i увеличиваются разряды чисел

        plasces_summ = x.arr[i] + y.arr[i] + carry; // в переменную суммы разрядов записывается сумма и значение переноса с прошлого складывания
        summ -> arr[i] = plasces_summ%100;          // в ячейку результата записывается одно или двузначное число
        carry = (int)(plasces_summ/100);            // если результат складывания >= 100 в значение переноса записывается 1
                                                    // так как максимальная сумма двух разрядов это 99+99 == 198, 198/100 == 1
    }

    return *summ;
}

uint1024_t subtr_op(uint1024_t x, uint1024_t y) {   // функция разности двух чисел типа uint1024_t 

    uint1024_t *subtr;
    subtr = calloc(1, 155);
    unsigned char carry = 0;       // то что называется "занимаем 1" при вычитании столбиком  
    signed char places_subtr = 0;  // разность двух разрядов чисел
    unsigned char y_start = 154;
    unsigned char index = 0;

    while (y.arr[y_start] == 0) {
        y_start--;
    }

    while (index <= y_start) {
        
        places_subtr = x.arr[index] - y.arr[index] - carry; // результат равен разности разрядов минус то что было занято на прошлом шаге

        if (places_subtr < 0) {                     // если разность разрядов меньше нуля необходимо занять 1 из более старшего разряда
            places_subtr += 100; 
            carry = 1;                              // записываем занятую 1 
        }

        subtr -> arr[index] = places_subtr;
        index++;
    }

    for (unsigned char j = index; j < 154; j++) {
        subtr -> arr[j] = x.arr[j];
    } 


    return *subtr;

}


uint1024_t mult_op(uint1024_t x, uint1024_t y) { // функция умножения двух чисел типа uint1024_t
    
    uint1024_t *mult;
    mult = calloc(1, 155);
    unsigned char carry = 0;      // значение переноса в следующий разряд числа mult от произведения разряда числа x на предыдущий разряд y (0 <= carry <= 99)
    unsigned char x_start = 154;  // x_start и y_start максимальные индексы в структуре
    unsigned char y_start = 154;
    int places_mult = 0;          // произведение двух разрядов хранится в int так как максимально оно 99*99 == 9801

    while (x.arr[x_start] == 0) { // так как разряды хранятся от младшего к страшему, дойдя до первого ненулевого элемента структуры 
        x_start--;                // найдем индекс самого старшего разряда числа
    }

    while (y.arr[y_start] == 0) { // аналогичная операция для второго числа
        y_start--;
    }

    for (int i = 0; i <= x_start; i++) { // разряд первого числа будем умножать на каждый разряд второго числа и перезаписывать результат в mult

        for (int j = 0; j <= y_start + 1; j++) {

            places_mult = x.arr[i]*y.arr[j] + carry;                      // умножение столбиком
            mult -> arr[i + j] += places_mult % 100;                          // добавляем последние два числа places_mult как двузначаное числа в элемент mult[i+j]
            carry = (int)(mult -> arr[i + j] / 100) + (int)(places_mult/100); // в перенос запишем "лишнее" в mult[i+j] и то что перенесется из places_mult
            mult -> arr[i + j] = (mult -> arr[i + j]) % 100;                    // убираем лишнее из mult[i+j] (делаем его <100)
            
        }
        

    }

    return *mult;

}


void printf_value(const uint1024_t* x) { // функция вывода числа uint1024_t в поток

    int j = 154;

    while (x->arr[j] == 0 && j > -1) { // проходимся до самого старшего разряда числа (печатать его необходимо от старшего разряда к младшему)
        j--;
    }
 
    if (j == -1) { // если j == -1 значит все разряды равны нулю, значит само число равно нулю
        printf("0\n");
        return;
    }

    for (int i = j; i > -1; i--) { // иначе начиная со старшего разряда будем выводить разряд

        if ((int)((x ->arr[i])/10) == 0 && i!= j && i !=0 ) { // если в разряде хранится однозначное число при этом разряд не первый и не последний

            printf("0%d", x -> arr[i]);                       // значит нужно добавить перед ним ноль чтобы обеспечить правильный вывод

        }

        else if (i == 0) {                    // если дошли до самого младшего разряда - необходимо добавить после него \n

            if (x -> arr[i] / 10 == 0 && j != 0) {
                printf("0%d\n", x->arr[i]); // если младший разряд представлен однозначным числом - так же добавляем ноль перед ним
            }
            else {
                printf("%d\n", x->arr[i]);
            }

        }

        else {
            printf("%d", x->arr[i]); // если ни одно из условий выше не выполнилось, значит просто выводим двузначное число хранящееся в ячейке
        }

    }

}


void scanf_value(uint1024_t* x) { // функция считывания числа из стандартного потока ввода

    char string_in[309];     // во входящей строке максимум может быть 309 цифр т.к в 2**1024 - 1 будет 309 цифр
    scanf("%s", &string_in); // запишем строку в переменную 
    int index = 0;
    int digit = 0;
    int temp_sum = 0;
    int end_index = 0;

    while (string_in[end_index] != '\0') { // найдем конец строки, дойдя до нуль терминатора
        end_index++;
    }    

    for (int i = end_index-1; i > -1; i--) { // пройдемся с конца строки в начало - с младшего разряда числа к старшему

        char symbol = string_in[i]; // считаем символ
        char temp_string[2];        // записываем во временную строку из 2 сиволов
        temp_string[0] = symbol;    // первым символом будет цифра числа
        temp_string[1] = '\0';      // вторым символом поставим нуль терминатор
        digit = atoi(temp_string);  // преобразуем строку в цисло

        if (i%2 == (end_index-1)%2 && i != 0) { // будем преобразовываать две цифры в двузначное число вида temp_sum = 10*a + b
            temp_sum += digit;                  // если цифра имеет такой же остаток от деления на 2 как и самая младшая цифра, то temp_sum += b
        }

        else if (i == 0 && temp_sum!=0) { // дойдя до самой старшей цифре числа, проверим что она является 'a' в temp_sum и домножим на 10
            temp_sum += digit*10;
            x -> arr[index] = temp_sum;   // это последний - самый старший разряд числа
            temp_sum = 0;
        }

        else if (i == 0 && temp_sum == 0) { // если самая старшая цифра числа является 'b' в temp_sum, то просто запишем ее последнюю ячейку
            temp_sum += digit;
            x -> arr[index] = temp_sum;
            temp_sum = 0;
        }

        else {
            temp_sum += digit * 10;     // если же цифра не последняя и имеет не такой остаток от деления индекса на 2 как младшая
            x -> arr[index] = temp_sum; // то она является 'a' в temp_sum
            temp_sum = 0;               // значит элемент структуры заполнен - переносим результат в ячейку структуры и обнуляем temp_sum
            index++;
        }
        
    }

}
