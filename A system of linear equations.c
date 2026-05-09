// ansi 1251
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define EPS 1e-9

////////// Заверешение программы (0) //////////
void close(bool *active) {
	if (active == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	int status;
	char end;
	
	printf("Вы уверены, что хотите закрыть программу? (1 - да, 0 - нет): ");
	if (scanf("%d%c", &status, &end) != 2 || status != 1 || end != '\n') {
		return;
	}
	
	*active = 0;
}

////////// Функция ввода имени файла //////////
void input_file(char **file_name, int *error) {
	int length;
	char *new_name = NULL;
	char ch;
	
	length = 0;
	printf("Введите относительное или абсолютное имя файла: ");
	while(1) {
		new_name = realloc(*file_name, sizeof(char) * (length+1));
		if (new_name == NULL) {
			printf("Критическая ошибка!");
			free(*file_name);
			*error = 1;
			return;
		} else {
			*file_name = new_name;
		}
		
		ch = getchar();
		if (ch == '\n') {
			(*file_name)[length] = '\0';
			break;
		}
		
		(*file_name)[length] = ch;
		length++;
	}
}

////////// Функции, которые добавляют в массив новую строку //////////
// добавление новых номеров в промежуточную базу
int add_numbers(int **unique_numbers, int *cols, double ***numbers, int total_variables) {
	int i, k;
	bool isnt;
	int *new_unique_numbers = NULL;
	
	if (unique_numbers == NULL || cols == NULL || numbers == NULL) {
		return 0;
	}
	
	for (i = 0; i < total_variables; i++) {
		isnt = 1;
		for (k = 0; isnt && k < *cols; k++) {
			if ((*numbers)[i][1] == (*unique_numbers)[k]) {
				isnt = 0;
			}
		}
		if (isnt) {
			new_unique_numbers = realloc(*unique_numbers, sizeof(int) * (*cols+1));
			if (new_unique_numbers == NULL) {
				return -1;
			} else {
				*unique_numbers = new_unique_numbers;
				
				(*unique_numbers)[*cols] = (*numbers)[i][1];
				*cols = *cols + 1;
			}
		}
	}
	
	return 1;
}

// сортирует уникальные номера неизвестных в порядке возрастания,
// вместе с принадлежащими им столбцами
bool sort_matrix(double ***matrix, int **unique_numbers,
				 int new_cols, int rows) {
	int i, j, k;
	int intermediate_int;
	double intermediate_value;
	
	if (matrix == NULL || unique_numbers == NULL || unique_numbers == NULL) {
		return 0;
	}
	
	for (i = 0; i < new_cols; i++) {
		for (j = i+1; j < new_cols; j++) {
			if ((*unique_numbers)[j] < (*unique_numbers)[i]) {
				for (k = 0; k < rows; k++) {
					intermediate_value = (*matrix)[k][j];
					(*matrix)[k][j] = (*matrix)[k][i];
					(*matrix)[k][i] = intermediate_value;
				}
				
				intermediate_int = (*unique_numbers)[i];
				(*unique_numbers)[i] = (*unique_numbers)[j];
				(*unique_numbers)[j] = intermediate_int;
			}
		}
	}
	
	return 1;
}

// удаление динамических массивов в add_row_matrix()
void clear_data_add_row_matrix(int new_rows, double ***new_matrix) {
	int i;
	
	if (*new_matrix != NULL) {
		for (i = 0; i < new_rows; i++) {
			free((*new_matrix)[i]);
		}
	
		free(*new_matrix);
	}
}

// обновляет матрицу, записывая новую строку
void add_row_matrix(double ***matrix, int **unique_numbers, int *cols, int *rows,
					double **equalities, double ***numbers, int total_variables,
					double equated, int *error) {
	int i, k;
	
	int *new_unique_numbers = NULL;
	double **new_matrix = NULL;
	double *new_row = NULL; // переменная,которая используется при увеличении длины строк матриц после доабвления новых уникальных номеров
	double **new_matrix_row = NULL; // переменная, которая использутечя при добавлении новой строки в конец матрицы
	int new_cols = *cols;
	int new_rows = *rows;
	double *new_equalities = NULL;
	
	if (matrix == NULL || unique_numbers == NULL || cols == NULL || rows == NULL || equalities == NULL || numbers == NULL) {
		printf("Критическая ошибка\n");
		clear_data_add_row_matrix(new_rows, &new_matrix);
		*error = 1;
		return;
	}
	
	// копируем старую версию массива уникальных номеров
	new_unique_numbers = calloc(new_cols, sizeof(double));
	if (new_unique_numbers == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		clear_data_add_row_matrix(new_rows, &new_matrix);
		*error = 1;
		return;
	} else {
		for (i = 0; i < new_cols; i++) {
			new_unique_numbers[i] = (*unique_numbers)[i];
		}
	}	
	
	// добавляем в старую версию массива с номерами неизвестных новые
	switch (add_numbers(&new_unique_numbers, &new_cols, numbers, total_variables)) {
		case -1:
			printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
			clear_data_add_row_matrix(new_rows, &new_matrix);
			*error = 1;
			return;
		case 0:
			printf("Ошибка!\n");
			clear_data_add_row_matrix(new_rows, &new_matrix);
			*error = 1;
			return;
	}
	
	// копирум старую версию матрицы, учитывая новые номера неизвестных, добавляем новую строку в неё и заполняем нулями
	new_matrix = calloc((new_rows+1), sizeof(double*));
	if (new_matrix == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		clear_data_add_row_matrix(new_rows, &new_matrix);
		*error = 1;
		return;
	} else {
		for (i = 0; i < new_rows+1; i++) {
			new_matrix[i] = calloc(new_cols, sizeof(double));
			if (new_matrix[i] == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_add_row_matrix(new_rows, &new_matrix);
				*error = 1;
				return;
			}
		}
		
		for (i = 0; i < new_rows; i++) {
			for (k = 0; k < *cols; k++) {
				new_matrix[i][k] = (*matrix)[i][k];
			}
			for (k = *cols; k < new_cols; k++) {
				new_matrix[i][k] = 0;
			}
		}
		
		for (i = 0; i < new_cols; i++) {
			new_matrix[new_rows][i] = 0;
		}
		
		new_rows++;
	}
	
	//соритируем копию массива с уникальными номерами в порядке возрастания
	if (!sort_matrix(&new_matrix, &new_unique_numbers, new_cols, *rows)) {
		printf("Ошибка!\n");
		clear_data_add_row_matrix(new_rows, &new_matrix);
		*error = 1;
		return;
	}
	
	// записываем значения в новую строку матрицы
	for (i = 0; i < total_variables; i++) {
		for (k = 0; k < new_cols; k++) {
			if ((*numbers)[i][1] == new_unique_numbers[k]) {
				new_matrix[new_rows-1][k] = (*numbers)[i][0];
				break;
			}
		}
	}
	
	/*// вывод матрицы (для разработчика)
	printf("%d %d %d %d\n", *cols, new_cols, *rows, new_rows);
	for (i = 0; i < new_cols; i++) {
		printf("%d ", new_unique_numbers[i]);
	}
	printf("\n");
	for (i = 0; i < new_rows; i++) {
		for (k = 0; k < new_cols; k++) {
			printf("%lf ", new_matrix[i][k]);
		}
		printf("\n");
	}*/
	
	// обновляем массив со свободными членами
	new_equalities = realloc(*equalities, sizeof(double) * new_rows);
	if (new_equalities == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		clear_data_add_row_matrix(new_rows, &new_matrix);
		*error = 1;
		return;
	} else {
		*equalities = new_equalities;
		(*equalities)[new_rows-1] = equated;
	}
	
	// очищаем матрицу matrix
	for (i = 0; i < *rows; i++) {
		free((*matrix)[i]);
	}
	free(*matrix);
	
	
	// очищаем массив unique_numbers
	free(*unique_numbers);
	
	// заменяем страык указатели на новые
	*matrix = new_matrix;
	*unique_numbers = new_unique_numbers;
	*rows = new_rows;
	*cols = new_cols;
}

////////// Функции, которые занимаются парсингом строки //////////
// проверка того, что символ является цифрой от 1 до 9 включительно
bool is_number(const char ch) {
	return ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5'
		   || ch == '6' || ch == '7' || ch == '8' || ch == '9';
}

// проверка на допустимость символа в уравнении
bool is_allowed(const char ch) {
	return is_number(ch) || ch == '0' || ch == '-' || ch == '+' || ch == '.' || ch == ' '
		   || ch == 'x' || ch == 'X' || ch == '\n' || ch == '=' || ch == ',';
}

// удаление лишних пробелов
bool delete_space(char **str, int *i, int *n) {
	if (str == NULL || i == NULL || n == NULL) {
		printf("Критическая ошибка!");
		return 1;
	}
	
	int k = *i;
	char *new_str = calloc(*n-1, sizeof(char));
	
	if (new_str == NULL) {
		printf("Критическая ошибка!");
		return 1;
	}
	
	while (k < *n) {
		(*str)[k] = (*str)[k+1];
		k++;
	}
	
	*n = *n - 1;
	*i = *i - 1;
	
	for (k = 0; k < *n; k++) {
		new_str[k] = (*str)[k];
	}
	
	free(*str);
	*str = new_str;
	
	return 0;
}

// удаление динамических массивов в read_eqauation()
void clear_data_read_equation(char **str_number, double ***numbers, int total_variables) {
	int i;
	
	if (*str_number != NULL) {
		free(*str_number);
	}
			
	if (*numbers != NULL) {
		for (i = 0; i < total_variables; i++) {
			free((*numbers)[i]);
		}
		
		free(*numbers);
	}
}

// проверка на наличие номера неизвестной
bool count_numbers(double ***numbers, int total_variables, double num) {
	int i;
	for (i = 0; i < total_variables; i++) {
		if ((*numbers)[i][1] == num) {
			return 0;
		}
	}
	
	return 1;
}

// чтение строки
void read_equation(double ***matrix, char **str, const int *len, double **equalities,
				   int **unique_numbers, int *cols, int *rows, int *error) {
	if (matrix == NULL || str == NULL || len == NULL || equalities == NULL ||
		unique_numbers == NULL || cols == NULL || rows == NULL) {
		printf("Критическая ошибка!");
		return;
	}
	
	// переменные для чтения числа
	char *str_number = NULL, *new_str_number = NULL;
	char *end;
	int str_number_len;
	double number;
	
	// переменные для массива коэффициентов при неизвестных и их номеров
	// 0 - численное значение коэффициента при неизвестной
	// 1 - номер неизвестной
	double **numbers = NULL, **new_numbers = NULL, *new_number = NULL;
	int total_variables;
	
	// вспомогательные переменные для чтения строки
	int n = *len, i, j, k;
	bool is_num, right, start;
	
	if ((*str)[0] == '=') {
		printf("Некорректный ввод уравнения\n");
		*error = 1;
		return;
	}
	
	// форматирование и первая проверка строки
	for (i = 0; i < n; i++) {
		// проверка на нахождении в строке посторонних символов
		if (is_allowed((*str)[i]) == 0) {
			printf("В уравнении есть недопустимый символ\n");
			*error = 1;
			return;
		}
		// замена ',' на '.'
		if ((*str)[i] == ',') {
			(*str)[i] = '.';
		}
		// замена 'X' на 'x'
		if ((*str)[i] == 'X') {
			(*str)[i] = 'x';
		}
		// удаление всех пробелов
		if ((*str)[i] == ' ' || (*str)[i] == '\t') {
			if (delete_space(str, &i, &n)) {
				return;
			}
		}
	}
	
	// проверка на пустоту строки после форматирования
	if (n == 0) {
		printf("Строка пустая\n");
		*error = 1;
		return;
	}
	
	// чтение строки
	for (i = 0, str_number_len = 0, start = 1,
		 is_num = 1, number = 0, right = 0, total_variables = 0; i < n; i++) {
		
		// проверка на то, что номер неизвестной целое число
		if (is_num == 0 && (*str)[i] == '.' && right == 0) {
			printf("Номер неизвестной должен быть натуральным числом\n");
			*error = 1;
			clear_data_read_equation(&str_number, &numbers, total_variables);
			return;
		}
		
		// действия во время начала чтения строки
		if (start) {
			start = 0;	
			
			if ((*str)[i] == 'x') {
				is_num = 0;
				
				new_numbers = calloc(total_variables+1, sizeof(double*));
				
				if (new_numbers == NULL) {
					printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
					free(str_number);
					*error = 1;
					return;
				} else {
					numbers = new_numbers;
					
					new_number = calloc(2, sizeof(double));
					
					if (new_number == NULL) {
						printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
						free(str_number);
						free(numbers);
						*error = 1;
						return;
					} else {
						numbers[0] = new_number;
					
						numbers[total_variables][0] = 1;
						numbers[total_variables][1] = -1;
						total_variables++;
						//printf("Коэффициент: %lf\n", numbers[total_variables-1][0]);
					}
				}
				
				continue;
			} else {
				new_str_number = calloc(1, sizeof(char));
			
				if (new_str_number == NULL) {
					printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
					*error = 1;
					return;
				} else {
					str_number = new_str_number;
					
					str_number[0] = (*str)[i];
					str_number_len = 1;
				}
			}
			
			continue;
		}
		
		// определние начала правой части уравнения
		if ((*str)[i] == '=' && is_num == 0 && (*str)[i-1] != 'x') {
			if (i == n-1) {
				printf("Уравнение ни к чему не приравненно\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			}
			
			right = 1;
			
			new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				
				str_number[str_number_len] = '\0';
				str_number_len = 0;
				number = strtod(str_number, &end);
			}
			
			if (count_numbers(&numbers, total_variables-1, number) == 0) {
				printf("Номера неизвестных повторяются\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			}
			
			numbers[total_variables-1][1] = number;
			
			if (n-i != 2) {
				new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
				if (new_str_number == NULL) {
					printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
					*error = 1;
					clear_data_read_equation(&str_number, &numbers, total_variables);
					return;
				} else {
					str_number = new_str_number;
					
					str_number[0] = (*str)[++i];
					str_number_len = 1;
				}
			}
			
			continue;
		}
		
		// конец чтения номера неизвестной
		if (right == 0 && ((*str)[i] == '+' || (*str)[i] == '-') && is_num == 0) {	
			//определяем номер неизвестной
			if (str_number_len == 0) {
				printf("У неизвестной нет номера\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			}
			
			is_num = 1;	
			
			new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				
				str_number[str_number_len] = '\0';
				str_number_len = 0;
				number = strtod(str_number, &end);
			}
			
			if (number <= 0) {
				printf("Номер неизвестной должен быть натуральным числом\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			}
			
			if (count_numbers(&numbers, total_variables, number) == 0) {
				printf("Номера неизвестных повторяются\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			}
			
			// записываем номер неизвестной в массив numbers
			numbers[total_variables-1][1] = number;
			
			// начинаем чтение нового коэффициента со знака (+/-)
			new_str_number = realloc(str_number, sizeof(char));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				
				str_number[0] = (*str)[i];
				str_number_len = 1;
			}
			
			//printf("Номер: %lf\n", numbers[total_variables-1][1]);
			continue;
		}
		
		// конец чтения коэффициента при неизвестной
		if (right == 0 && (*str)[i] == 'x' && is_num == 1) {
			is_num = 0;
			
			new_numbers = realloc(numbers, sizeof(double*) * (total_variables+1));
				
			if (new_numbers == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				numbers = new_numbers;
				
				new_number = calloc(2, sizeof(double));
				
				if (new_number == NULL) {
					printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
					clear_data_read_equation(&str_number, &numbers, total_variables);
					*error = 1;
					return;
				} else {
					numbers[total_variables] = new_number;
					
					total_variables++;
				}
			}
			
			new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				str_number[str_number_len] = '\0';
			}
			
			// определяем коэффициент
			if (str_number_len == 1) {
				if ((*str)[i-1] == '+') {
					number = 1;
				} else if ((*str)[i-1] == '-') {
					number = -1;
				} else {
					number = strtod(str_number, &end);
				}
			} else {
				number = strtod(str_number, &end);
			}
			
			str_number_len = 0;
			free(str_number);
			str_number = NULL;
			
			// записываем коэффициент в массив numbers
			numbers[total_variables-1][0] = number;
			//printf("Коэффициент: %lf\n", numbers[total_variables-1][0]);
			
			/* нужно, чтобы при опеределнии повторяющихся переменных её не зацепило
			   поскольку номера - натуральные числа, то -1 точно не появится*/
			numbers[total_variables-1][1] = -1;
			
			continue;
		}
		
		// проверка правой части уравнения на посторонние символы
		if (right == 1 && ((*str)[i] == 'x' || (*str)[i] == '=')) {
			printf("Некорректный ввод уравнения\n");
			clear_data_read_equation(&str_number, &numbers, total_variables);
			*error = 1;
			return;
		}
		
		// проверка символа
		if (is_number((*str)[i]) || (*str)[i] == '0' || (*str)[i] == '.') {
			new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				//printf("Символ: %c\n", (*str)[i]);
				
				str_number[str_number_len] = (*str)[i];
				str_number_len++;
				
				// проверка на начало числа с нуля
				if (str_number_len <= 1 && (*str)[i] == '0' && is_number((*str)[i+1])) {
					printf("Число не может начинаться с нуля\n");
					*error = 1;
					clear_data_read_equation(&str_number, &numbers, total_variables);
					return;
				}
			}
		} else {
			printf("Некорректный ввод уравнения\n");
			clear_data_read_equation(&str_number, &numbers, total_variables);
			*error = 1;
			return;
		}
		
		// конец чтения строки
		if ((*str)[i+1] == '\0' && right == 1) {
			new_str_number = realloc(str_number, sizeof(char) * (str_number_len+1));
			if (new_str_number == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				clear_data_read_equation(&str_number, &numbers, total_variables);
				*error = 1;
				return;
			} else {
				str_number = new_str_number;
				
				str_number[str_number_len] = '\0';
				number = strtod(str_number, &end);
				//printf("Свободный член: %lf\n", number);
				
				free(str_number);
				str_number = NULL;
			}
			
			// добавляем в матрицу новую строку
			add_row_matrix(matrix, unique_numbers, cols, rows, equalities, &numbers,
						   total_variables, number, error);
			clear_data_read_equation(&str_number, &numbers, total_variables);
			
			return;
		}
	}
}

////////// Функция, добавляющая уравнение через консоль (1) //////////
void add_console(double ***matrix, int **unique_numbers, double **equalities,
				 int *cols, int *rows) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка");
		return;
	}
	
	char *str = NULL;
	char *new_str = NULL;
	int n;
	int error;
	char ch;
	
	printf("Введите уравнение в формате a1xn1 + a2xn2 + ... + aixnk = b, где\n"
		   "a1, a2, ..., ai - коэффициенты при неизвестных, вещественные числа\n"
		   "n1, n2, ..., nk - номера неизвестных, натуральные числа\n"
		   "b - вещественное число\n");
	printf("-> ");
	
	n = 0;
	while(1) {
		new_str = realloc(str, sizeof(char) * (n+1));
		if (new_str == NULL) {
			printf("Критическая ошибка");
			free(str);
			return;
		} else {
			str = new_str;
		}
		
		ch = getchar();
		if (ch == '\n') {
			str[n] = '\0';
			break;
		}
		
		str[n] = ch;
		n++;
	}
	
	error = 0;
	read_equation(matrix, &str, &n, equalities, unique_numbers, cols, rows, &error);
	free(str);
}

////////// Функция, которая удаляет все данные (3) //////////
void del_all(double ***matrix, int **unique_numbers, double **equalities,
			 int *cols, int *rows) {
	int i;
	char end;
	
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL ||
		cols == NULL || rows == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (*rows == 0) {
		printf("Уравнений в системе нет");
		return;
	}
	
	printf("Вы уверены, что хотите удалить все уравнения? (1 - да, 0 - нет): ");
	if (scanf("%d%c", &i, &end) != 2 || i != 1 || end != '\n') {
		return;
	}
	
	free(*unique_numbers);
	*unique_numbers = NULL;
	free(*equalities);
	*equalities = NULL;
	for (i = 0; i < *rows; i++) {
		free((*matrix)[i]);
	}
	free(*matrix);
	*matrix = NULL;
	*rows = 0;
	*cols = 0;
}

////////// Функция, которая вставляет уравнение в СЛАУ (4) //////////
void insert(double ***matrix, int **unique_numbers, double **equalities,
		 	int *cols, int *rows) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL ||
		cols == NULL || rows == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (*rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	int row, i, k;
	char end;
	double *intermediate_poiter = NULL;
	double intermediate_value;
	
	printf("Введите номер интересующей вас позиции: ");
	if (scanf("%d%c", &row, &end) != 2 || row <= 0 || row > *rows || end != '\n') {
		printf("Вставить уравнение на эту позицию нельзя\n");
		return;
	}
	row--;
	
	// добавление нового уравнения в конец СЛАУ
	add_console(matrix, unique_numbers, equalities, cols, rows);
	
	// перемещение последнего уравнения на выбранную позицию
	for (i = *rows-1; i > row; i--) {
		//перестановко коэффициентов
		intermediate_poiter = (*matrix)[i];
		(*matrix)[i] = (*matrix)[i-1];
		(*matrix)[i-1] = intermediate_poiter;
		
		// перестановка свободных членов
		intermediate_value = (*equalities)[i];
		(*equalities)[i] = (*equalities)[i-1];
		(*equalities)[i-1] = intermediate_value;
	}
}

////////// Функция, загружающая СЛАУ из файла (6) //////////
void loading(double ***matrix, int **unique_numbers, double **equalities,
			 int *cols, int *rows) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL ||
		cols == NULL || rows == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	char *str = NULL, *new_str = NULL;
	char *name = NULL;
	int n;
	int error = 0;
	char ch;
	FILE *file;
	
	int i, j;
	double **copy_matrix = NULL;
	double *copy_row = NULL;
	double *copy_equalities = NULL;
	int *copy_unique_numbers = NULL;
	int new_cols = *cols;
	int new_rows = *rows;
	
	// делаем копии массивов
	copy_matrix = calloc(*rows, sizeof(double*));
	if (copy_matrix == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < *rows; i++) {
			copy_row = calloc(*cols, sizeof(double));
		
			if (copy_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free(copy_matrix[j]);
				}
				free(copy_matrix);
				return;
			} else {
				copy_matrix[i] = copy_row;
			}
		}
	}
	
	copy_equalities = calloc(*rows, sizeof(double));
	if (copy_equalities == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		for (j = 0; j < *rows; j++) {
			free(copy_matrix[j]);
		}
		free(copy_matrix);
		return;
	}
	
	copy_unique_numbers = calloc(*cols, sizeof(int));
	if (copy_unique_numbers == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		for (j = 0; j < *rows; j++) {
			free(copy_matrix[j]);
		}
		free(copy_matrix);
		free(copy_equalities);
		return;
	}
	
	for (i = 0; i < *rows; i++) {
		for (j = 0; j < *cols; j++) {
			copy_matrix[i][j] = (*matrix)[i][j];
		}
	}
	
	for (i = 0; i < *rows; i++) {
		copy_equalities[i] = (*equalities)[i];
	}
	
	for (i = 0; i < *cols; i++) {
		copy_unique_numbers[i] = (*unique_numbers)[i];
	}
	
	input_file(&name, &error);
	if (error) {
		printf("Критическая ошибка!");
		free(name);
		for (j = 0; j < *rows; j++) {
			free(copy_matrix[j]);
		}
		free(copy_matrix);
		free(copy_equalities);
		free(copy_unique_numbers);
		return;
	}
	
	file = fopen(name, "rt");
	free(name);
	
	if (file == NULL) {
		printf("Не удалось открыть файл\n");
		return;
	}
	
	n = 0;
	while (!feof(file)) {
		new_str = realloc(str, sizeof(char) * (n+1));
		if (new_str == NULL) {
			printf("Критическая ошибка!");
			fclose(file);
			free(str);
			
			for (j = 0; j < new_rows; j++) {
				free(copy_matrix[j]);
			}
			free(copy_matrix);
			free(copy_equalities);
			free(copy_unique_numbers);
			return;
		} else {
			str = new_str;
		}
		
		ch = fgetc(file);
		if (ch == '\n' || ch == -1) {
			str[n] = '\0';
			read_equation(&copy_matrix, &str, &n, &copy_equalities, &copy_unique_numbers, &new_cols, &new_rows, &error);
			free(str);
			str = NULL;
			if (error) {
				for (j = 0; j < *rows; j++) {
					free(copy_matrix[j]);
				}
				free(copy_matrix);
				free(copy_equalities);
				free(copy_unique_numbers);
				return;
			}
			n = 0;
			continue;
		}
		str[n] = ch;
		n++;
	}
	
	for (i = 0; i < *rows; i++) {
		free((*matrix)[i]);
	}
	free(*matrix);
	*matrix = copy_matrix;
	
	free(*equalities);
	*equalities = copy_equalities;
	
	free(*unique_numbers);
	*unique_numbers = copy_unique_numbers;
	
	*cols = new_cols;
	*rows = new_rows;
	
	fclose(file);
}

////////// Функция, меняющая местами уравнения (5) //////////
void swap(double ***matrix, double **equalities, int cols, int rows) {
	if (matrix == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	int first, second, i;
	double intermediate_value;
	char end;
	
	printf("Введите через пробел номера двух уравнений: ");
	if (scanf("%d%d%c", &first, &second, &end) != 3 || first <= 0 ||
		first > rows || second <= 0 || second > rows || first == second || end != '\n') {
		printf("Уравнения с такими номерами нельзя поменять местами\n");
		return;
	}
	
	for (i = 0; i < cols; i++) {
		intermediate_value = (*matrix)[second-1][i];
		(*matrix)[second-1][i] = (*matrix)[first-1][i];
		(*matrix)[first-1][i] = intermediate_value;
	}
	
	intermediate_value = (*equalities)[second-1];
	(*equalities)[second-1] = (*equalities)[first-1];
	(*equalities)[first-1] = intermediate_value;
}

////////// Функция, выводящая одно уравнение в консоль (7) //////////
void print_console_once(double ***matrix, int **unique_numbers, double **equalities,
						int cols, int rows, int r, FILE *file) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	int row = r, i, k, zero;
	char end;
	
	if (row == -1) {
		printf("Введите номер интересующего вас уравнения: ");
		if (scanf("%d%c", &row, &end) != 2 || row <= 0 || row > rows || end != '\n') {
			printf("Такого уравнения не существует\n");
			return;
		}
	}
	
	for (i = 0, k = 0, zero = 0; i < cols; i++) {
		if ((*matrix)[row-1][i] != 0) {
			if (k == 0) {
				fprintf(file, "%lfx%d ", (*matrix)[row-1][i], (*unique_numbers)[i]);
				k++;
				continue;
			}
			if ((*matrix)[row-1][i] > 0) {
				fprintf(file, "+ %lfx%d ", (*matrix)[row-1][i], (*unique_numbers)[i]);
			} else {
				fprintf(file, "- %lfx%d ", fabs((*matrix)[row-1][i]), (*unique_numbers)[i]);
			}
			k++;
		} else {
			zero++;
				
			if (zero == cols) {
				fprintf(file, "%lf ", 0);
			}
		}
	}
	fprintf(file, "= %lf\n", (*equalities)[row-1]);
}

////////// Функция, выводящая СЛАУ в консоль (8) //////////
void print_console_all(double ***matrix, int **unique_numbers, double **equalities,
						int cols, int rows, FILE *file) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	int i;
	for (i = 0; i < rows; i++) {
		fprintf(file, "%d: ", i+1);
		print_console_once(matrix, unique_numbers, equalities, cols, rows, i+1, file);
	}
}

////////// Функция, которая удаляет одно уравнение (2) //////////
void del(double ***matrix, int **unique_numbers, double **equalities,
		 int *cols, int *rows) {
	double **copy_matrix = NULL;
	double *copy_row = NULL;
	int *copy_unique_numbers = NULL;
	
	int row, i, j, k;
	char end;
	bool is_zero;
	int new_cols = *cols;
	
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL ||
		cols == NULL || rows == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (*rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	if ((*rows)-1 == 0) {
		free((*matrix)[0]);
		free(*matrix);
		free(*unique_numbers);
		free(*equalities);
		*matrix = NULL;
		*unique_numbers = NULL;
		*equalities = NULL;
		*cols = 0;
		*rows = 0;
		return;
	}
	
	printf("Введите номер уравнения: ");
	if (scanf("%d%c", &row, &end) != 2 || row <= 0 || row > *rows || end != '\n') {
		printf("Такого уравнения не существует\n");
		return;
	}
	row--;
	
	// создаём копию матрицы коэффициентов
	copy_matrix = calloc(*rows-1, sizeof(double*));
	if (copy_matrix == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0, k = 0; i < *rows; i++) {
			if (i == row) {
				k = 1;
				continue;
			}
			copy_row = calloc(*cols, sizeof(double));
			if (copy_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free(copy_matrix[j]);
				}
				free(copy_matrix);
				return;
			} else {
				copy_matrix[i-k] = copy_row;
				
				for (j = 0; j < *cols; j++) {
					copy_matrix[i-k][j] = (*matrix)[i][j];
				}
			}
		}
	}
	
	// поиск лишних номеров неизвестных
	for (i = 0; i < new_cols; i++) {
		is_zero = 1;
		
		// определение столбца, все числа в котором - 0
		for (j = 0; j < *rows-1; j++) {
			if (copy_matrix[j][i] != 0) {
				is_zero = 0;
				break;
			}
		}
		
		// если столбец под номером i нулевой, то ...
		if (is_zero) {
			// делаем копию массива номеров неизвестных без лишнего номера
			copy_unique_numbers = malloc(sizeof(int) * (*cols-1));
			if (copy_unique_numbers == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < *rows-1; j++) {
					free(copy_matrix[j]);
				}
				free(copy_matrix);
				return;
			} else {
				for (j = 0, k = 0; j < *cols; j++) {
					if (j == i) {
						k = 1;
						continue;
					}
					copy_unique_numbers[j-k] = (*unique_numbers)[j];
				}
			}
			
			// удаляем столбец с нулями
			for (j = 0; j < *rows-1; j++) {
				// удаляем элемент в строке при помощи сдвигов
				for (k = i; k < *cols-1; k++) {
					copy_matrix[j][k] = copy_matrix[j][k+1];
				}
				
				// уменьшаем память для строки
				copy_row = realloc(copy_matrix[j], sizeof(double) * (*cols-1));
				if (copy_row == NULL) {
					for (k = 0; k < *rows-1; k++) {
						free(copy_matrix[k]);
					}
					free(copy_matrix);
					free(copy_unique_numbers);
					return;
				} else {
					copy_matrix[j] = copy_row;
				}
			}
			
			new_cols--;
		}
	}
	
	// очищаем старые массивы
	for (i = 0; i < *rows; i++) {
		free((*matrix)[i]);
	}
	free(*matrix);
	if (new_cols < *cols) {
		free(*unique_numbers);
	}
	
	// заменяем указатели на новые
	*matrix = copy_matrix;
	*rows = *rows - 1;
	if (new_cols < *cols) {
		*unique_numbers = copy_unique_numbers;
		*cols = new_cols;
	}
	
	print_console_all(matrix, unique_numbers, equalities, *cols, *rows, stdout);
}

////////// Функции, выводящие расширенную матрицу в консоль (10) //////////
// заполняет массив символов текстом (матрицой)
void print_matrix(double ***matrix, int cols, int rows, char ***text, int *width) {
	int size_number, number;
	int *sizes_of_columns = NULL;
	char *str_number = NULL;
	char *row = NULL;
	int full;
	int i, j, k;
	
	if (matrix == NULL || text == NULL || width == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	sizes_of_columns = calloc(cols, sizeof(int));
	if (sizes_of_columns == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < cols; i++) {
			sizes_of_columns[i] = 0;
		}
	}
	
	// определяем размеры столбцов матрицы по длине максимального числа в них
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			number = (int) (*matrix)[i][j];
			size_number = 7;
			if (number < 0) {
				size_number++;
				number = fabs(number);
			}
			if (number != 0) {
				while (number != 0) {
					number /= 10;
					size_number++;
				}
			} else {
				size_number++;
			}
			if (sizes_of_columns[j] < size_number) {
				sizes_of_columns[j] = size_number;
			}
		}
	}
		
	// вычисление ширины массива символов вместе с разделителями
	for (i = 0, *width = 0; i < cols; i++) {
		*width += sizes_of_columns[i];
	}
	
	// cols - пробелы-разделители (пробел после последнего числа превратится в '\0'
	*width += cols;
	
	// выделение памяти для массива символов
	*text = calloc(rows, sizeof(char*));
	if (*text == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		free(sizes_of_columns);
		return;
	} else {
		for (i = 0; i < rows; i++) {
			row = calloc(*width, sizeof(char));
			if (row == NULL) {
				free(sizes_of_columns);
				for (j = 0; j < i; j++) {
					free((*text)[j]);
				}
				free(*text);
				*text = NULL;
				*width = 0;
				return;
			} else {
				(*text)[i] = row;
			}
		}
	}
	
	// заполняем символьный массив числовой матрицой
	full = 0;
	for (i = 0; i < cols; i++) {
		str_number = calloc(sizes_of_columns[i]+1, sizeof(char));
		
		if (str_number == NULL) {
			printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
			free(sizes_of_columns);
			for (j = 0; j < rows; j++) {
				free((*text)[j]);
			}
			free(*text);
			*text = NULL;
			*width = 0;
			return;
		} else {
			for (j = 0; j < rows; j++) {
				size_number = snprintf(str_number, sizes_of_columns[i]+1, "%lf", (*matrix)[j][i]);
				for (k = 0; k < size_number; k++) {
					(*text)[j][k+full] = str_number[k];
				}
				for (k = size_number; k < sizes_of_columns[i]; k++) {
					(*text)[j][k+full] = ' ';
				}
				if (full+sizes_of_columns[i]+1 == *width) {
					(*text)[j][sizes_of_columns[i]+full] = '\0';
				} else {
					(*text)[j][sizes_of_columns[i]+full] = ' ';
				}
			}
		}
		
		full += sizes_of_columns[i]+1;
		free(str_number);
		str_number = NULL;
	}
	
	free(sizes_of_columns);
}

// вывод расширенной матрицы в консоль
void print_extended_matrix(double ***matrix, double **equalities, int **unique_numbers,
						   int cols, int rows, bool analysis) {
	int i, k;
	char **text = NULL;
	int width = 0;
	
	if (matrix == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	print_matrix(matrix, cols, rows, &text, &width);
	
	if (text != NULL) {
		for (i = 0; i < rows; i++) {
			for (k = 0; k < width; k++) {
				printf("%c", text[i][k]);
			}
			printf(" | %lf\n", (*equalities)[i]);
		}
	
		for (i = 0; i < rows; i++) {
			free(text[i]);
		}
		free(text);
	}
}

////////// Функция, выводящая основную матрицу в консоль (9) //////////
void print_main_matrix(double ***matrix, int cols, int rows) {
	int i, k;
	char **text = NULL;
	int width = 0;
	
	if (matrix == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	print_matrix(matrix, cols, rows, &text, &width);
	
	if (text != NULL) {
		for (i = 0; i < rows; i++) {
			for (k = 0; k < width; k++) {
				printf("%c", text[i][k]);
			}
			printf("\n");
		}
	
		for (i = 0; i < rows; i++) {
			free(text[i]);
		}
		free(text);
	}
}

////////// Функции, находящие трапецеидальный и нижнетреугольные виды матрицы (11 и 12) и ищущие ранги (18 и 19) //////////
// вывод матрицы с пронумерованными уравнениями и переменными сверху
void print_numbered_matrix(double ***step, int **unique_numbers,
						   int cols, int rows, bool extended, FILE *file) {
	char **text = NULL;
	int width;
	double **copy_step = NULL;
	double *new_row = NULL;
	int i, j;
	bool flag;
	int number, space;
	
	if (step == NULL || unique_numbers == NULL) {
		printf("Критическая ошибка!");
		return;
	}
	
	// выделение памяти для матрицы со строкой с номерами переменных
	copy_step = calloc(rows+1, sizeof(double*));
	if (copy_step == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		// первая строка с номерами переменных
		new_row = calloc(cols-1, sizeof(double));
		if (new_row == NULL) {
			printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
			free(copy_step);
			return;
		} else {
			copy_step[0] = new_row;
			for (i = 0; i < cols-1; i++) {
				copy_step[0][i] = (double) (*unique_numbers)[i];
			}
		}
		
		// копия переменных
		for (i = 1; i < rows+1; i++) {
			new_row = calloc(cols-1, sizeof(double));
			if (new_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free(copy_step[j]);
				}
				free(copy_step);
				return;
			} else {
				copy_step[i] = new_row;
				for (j = 0; j < cols-1; j++) {
					copy_step[i][j] = (*step)[i-1][j];
				}
			}
		}
	}
	
	// создание матрицы символов
	print_matrix(&copy_step, cols-1, rows+1, &text, &width);
	
	// удаление дробной части у номеров переменных
	for (i = 0, flag = 0; i < width; i++) {
		if (text[0][i] == '.') {
			flag = 1;
			text[0][i] = ' ';
			continue;
		}
		if (flag && text[0][i] != '0') {
			flag = 0;
		}
		if (flag) {
			text[0][i] = ' ';
		}
	}
	
	// расчёт места в 'шапке' таблицы под столбец с номерами уравнений
	space = 2; // ':', ' '
	number = rows;
	while (number) {
		space++;
		number /= 10;
	}
	
	// вывод матрицы
	for (i = 0; i < space; i++) {
		fprintf(file, " ");
	}
	for (i = 0; i < width; i++) {
		fprintf(file, "%c", text[0][i]);
	}
	fprintf(file, "\n");
	for (i = 1; i < rows+1; i++) {
		fprintf(file, "%d: ", i);
		for (j = 0; j < width; j++) {
			fprintf(file, "%c", text[i][j]);
		}
		if (extended) {
			fprintf(file, " | %lf\n", (*step)[i-1][cols-1]);
		} else {
			fprintf(file, "\n");
		}
	}
	
	for (i = 0; i < rows+1; i++) {
		free(copy_step[i]);
	}
	free(copy_step);
	
	for (i = 0; i < rows+1; i++) {
		free(text[i]);
	}
	free(text);
}

// приведение к ступенчатому виду
void step_type(double ***matrix, double **equalities, int **unique_numbers,
			   double ***step, int rows, int cols, bool analysis, FILE *file) {
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL || step == NULL) {
		printf("Критическая ошибка!");
		return;
	}
	
	int i, j, k, w; // итерационные переменные
	double first, second; // промежуточные значения
	int lead_col;
	double main_el_after;
	bool zero; // флаг
	double *row = NULL;
	
	// выделение памяти для расширенной матрицы step и её заполнение
	*step = calloc(rows, sizeof(double*));
	if (*step == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < rows; i++) {
			row = calloc(cols+1, sizeof(double));
			if (row == NULL) {		
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free((*step)[j]);
				}
				free(*step);
				return;
			} else {
				(*step)[i] = row;
				
				for (j = 0; j < cols; j++) {
					(*step)[i][j] = (*matrix)[i][j];
				}
				
				(*step)[i][cols] = (*equalities)[i];
			}
		}
	}
	
	if (analysis) {
		fprintf(file, "Исходная матрица:\n");
		print_numbered_matrix(step, unique_numbers, cols+1, rows, 1, file);
		fprintf(file, "\n");
	}
	
	// привведение матрицы к ступенчатому виду
	i = 0;
	lead_col = 0;
	while (i < rows && lead_col < cols+1) {
		if (fabs((*step)[i][lead_col]) < EPS) {
			if (analysis && i != cols) {
				fprintf(file, "Главный элемент при неизвестной №%d в строке №%d равен 0,"
				"то необходимо найти другую строке,"
				"коэффициент которой при неизвестной %d не равен 0 и поменять их местами\n",
				(*unique_numbers)[lead_col], i+1, (*unique_numbers)[lead_col]);
			}
			zero = 1;
			j = i+1;
			while (j < rows) {
				if (fabs((*step)[j][lead_col]) > EPS) {
					zero = 0;
					row = (*step)[i];
					(*step)[i] = (*step)[j];
					(*step)[j] = row;
					row = NULL;
					if (analysis && i != cols) {
						fprintf(file, "Поменяем местами строки №%d и №%d и получим следующую матрицу:\n", i+1, j+1);
						print_numbered_matrix(step, unique_numbers, cols+1, rows, 1, file);
						fprintf(file, "\n");
					}
					break;
				}
				j++;
			}
			if (zero) {
				lead_col++;
				if (analysis && i != cols) {
					fprintf(file, "Главные элементы у других уравнений также равны 0, "
					"тогда сделаем главным элементом коэффициент "
					"следующей неизвестной №%d в строке №%d\n",
					(*unique_numbers)[lead_col], i+1);
					print_numbered_matrix(step, unique_numbers, cols+1, rows, 1, file);
					fprintf(file, "\n");
				}
				continue;
			}
		}
		
		if (analysis && i != cols && i != cols-1) {
			fprintf(file, "Главный элемент в строке №%d - коэффициент у неизвестной №%d\n",
				   i+1, (*unique_numbers)[lead_col]);
			print_numbered_matrix(step, unique_numbers, cols+1, rows, 1, file);
			printf("\n");
		}
		first = (*step)[i][lead_col];
		for (j = i+1; j < rows; j++) {
			second = (-1)*(*step)[j][lead_col]/first;
			if (analysis && i != cols) {
				main_el_after = (*step)[j][lead_col];
				fprintf(file, "Умножим строку №%d на -(%lf/%lf) и сложим с №%d\n",
					   i+1, (*step)[j][lead_col], first, j+1);
			}
			for (k = lead_col; k < cols+1; k++) {
				if (analysis && i != cols) {
					fprintf(file, "A%d%d * -A%d%d/A%d%d + A%d%d = %lf * -(%lf/%lf) + %lf",
					i+1, k+1, j+1, lead_col+1, i+1, lead_col+1, j+1, k+1,
					(*step)[i][k], main_el_after, first, (*step)[j][k]);
				}
				(*step)[j][k] += (*step)[i][k]*second;
				if (fabs((*step)[j][k]) < EPS) {
					(*step)[j][k] = 0;
				}
				if (analysis && i != cols) {
					fprintf(file, " = %lf\n", (*step)[j][k]);
				}
			}
			if (analysis && i != cols) {
				fprintf(file, "\nПолучится матрица:\n");
				print_numbered_matrix(step, unique_numbers, cols+1, rows, 1, file);
				fprintf(file, "\n");
			}
		}
		
		lead_col++;
		i++;
	}
}

// поиск ранга
int rank(double ***step, int rows, int cols, bool analysis, bool mode) {
	int i, j;
	bool zero;
	int rank = rows;
	
	if (step == NULL) {
		return -1;
	}
	
	// определение ранга
	for (i = rows-1; i > 0; i--) {
		zero = 1;
		for (j = 0; j < cols; j++) {
			if (fabs((*step)[i][j]) > EPS) {
				zero = 0;
				break;
			}
		}
		if (zero) {
			rank--;
			continue;
		} else {
			break;
		}
	}
	
	if (analysis) {	
		if (mode == 0) {
			printf("Ненулевых строк в основной матрице %d, следовательно её ранг равен %d\n", rank, rank);
		} else {
			printf("Ненулевых строк в расширенной матрице %d, следовательно её ранг равен %d\n", rank, rank);
		}
	}
	
	return rank;
}

// вывод ранга в консоль
void print_rank(double ***matrix, double **equalities, int **unique_numbers, int cols, int rows,
				bool analysis) {
	double **step = NULL;
	int i, r = 0, R = 0;
					
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	// нахождение ступенчатого вида
	if (analysis) {
		printf("Поиск ступенчатого вида матрицы\n");
	}
	
	step_type(matrix, equalities, unique_numbers, &step, rows, cols, analysis, NULL);
	
	if (step != NULL) {
		r = rank(&step, rows, cols, analysis, 0);
		R = rank(&step, rows, cols+1, analysis, 1);
		
		if (r == -1) {
			printf("Критическая ошибка!\n");
			// освобождения памяти у step
			for (i = 0; i < R; i++) {
				free(step[i]);
			}
			free(step);
			return;
		}
		printf("Ранг r основной матрицы: %d\n", r);
		
		if (R == -1) {
			printf("Критическая ошибка!\n");
			// освобождения памяти у step
			for (i = 0; i < R; i++) {
				free(step[i]);
			}
			free(step);
			return;
		}
		printf("Ранг R расширенной матрицы: %d\n\n", R);
		
		if (R == r) {
			printf("Так как R = r, то матрица совместна\n");
		} else {
			printf("Так как R не равен r, то матрица несовместна\n");
		}
		
		// освобождения памяти у step
		for (i = 0; i < R; i++) {
			free(step[i]);
		}
		free(step);
	}
}

// вывод ступенчатого вида матрицы
void print_step_type(double ***matrix, double **equalities, int **unique_numbers,
					 int cols, int rows, bool analysis) {
	double **step = NULL;
	char **text = NULL;
	int width;
	int i, j, k, R;
	
	if (matrix == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	// нахождение ступенчатого вида
	step_type(matrix, equalities, unique_numbers, &step, rows, cols, analysis, stdout);
	
	if (step != NULL) {
		// определение ранга расширенной матрицы
		R = rank(&step, rows, cols+1, 0, 0);
		
		if (R == -1) {
			printf("Критическая ошибка!\n");
			for (i = 0; i < R; i++) {
				free(step[i]);
			}
			free(step);
			return;
		}
		
		// вывод матрицы в консоль
		print_matrix(&step, cols, R, &text, &width);
		if (text != NULL) {
			if (analysis) {
				printf("Трапецеидальный вид матрицы:\n");
			}
			
			for (i = 0; i < R; i++) {
				for (j = 0; j < width; j++) {
					printf("%c" , text[i][j]);
				}
				printf(" | %lf\n", step[i][cols]);
			}
			
			for (i = 0; i < R; i++) {
				free(step[i]);
			}
			free(step);
		
			for (i = 0; i < R; i++) {
				free(text[i]);
			}
			free(text);
		}
	}
}

// поиск нижнетреугольного вида матрицы
void lower_triangular_type(double ***matrix, int **unique_numbers,
						   double ***lower, int rows,
						   int cols, bool analysis, FILE *file) {
	if (matrix == NULL || unique_numbers == NULL || lower == NULL) {
		printf("Критическая ошибка!");
		return;
	}
	
	int i, j, k; // итерационные переменные
	double first, second; // промежуточные значения
	int lead_col;
	bool zero; // флаг
	double *row = NULL;
	double main_el_after;
	
	// выделение памяти для расширенной матрицы step и её заполнение
	*lower = calloc(rows, sizeof(double*));
	if (*lower == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < rows; i++) {
			row = calloc(cols, sizeof(double));
			if (row == NULL) {		
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free((*lower)[j]);
				}
				free(*lower);
				return;
			} else {
				(*lower)[i] = row;
				
				for (j = 0; j < cols; j++) {
					(*lower)[i][j] = (*matrix)[i][j];
				}
			}
		}
	}
	
	if (analysis) {
		printf("Исходная матрица:\n");
		print_numbered_matrix(lower, unique_numbers, cols+1, rows, 0, file);
		printf("\n");
	}
	
	i = rows-1;
    lead_col = cols-1;					
	while (i >= 0 && lead_col > 0) {
        if (fabs((*lower)[i][lead_col]) < EPS) {
			if (analysis) {
				printf("Главный элемент при неизвестной №%d в строке №%d равен 0, "
				"то необходимо найти другую строке,"
				"коэффициент которой при неизвестной %d не равен 0 и поменять их местами\n",
				(*unique_numbers)[lead_col], i+1, (*unique_numbers)[lead_col]);
			}
            zero = 1;
            for (j = i-1; j >= 0; j--) {
                if (fabs((*lower)[j][lead_col]) > EPS) {
                    zero = 0;
                    row = (*lower)[i];
                    (*lower)[i] = (*lower)[j];
                    (*lower)[j] = row;
                    row = NULL;
                    if (analysis) {
						printf("Поменяем местами строки №%d и №%d и получим следующую матрицу:\n", i+1, j+1);
						print_numbered_matrix(lower, unique_numbers, cols+1, rows, 0, file);
						printf("\n");
					}
                    break;
                }
            }
            if (zero) {
                lead_col--;
                if (analysis) {
					printf("Главные элементы у других уравнений также равны 0, "
					"тогда сделаем главным элементом коэффициент "
					"следующей неизвестной №%d в строке №%d\n",
					(*unique_numbers)[lead_col], i+1);
					print_numbered_matrix(lower, unique_numbers, cols+1, rows, 0, file);
					printf("\n");
				}
                continue;
            }
        }
        
        if (analysis && i != 0) {
			printf("Главный элемент в строке №%d - коэффициент у неизвестной №%d\n",
				   i+1, (*unique_numbers)[lead_col]);
			print_numbered_matrix(lower, unique_numbers, cols+1, rows, 0, file);
			printf("\n");
		}
        first = (*lower)[i][lead_col];
        for (j = i-1; j >= 0; j--) {
            second = (-1)*(*lower)[j][lead_col]/first;
            if (analysis) {
				main_el_after = (*lower)[j][lead_col];
				printf("Умножим строку №%d на -(%lf/%lf) и сложим с №%d\n",
					   i+1, (*lower)[j][lead_col], first, j+1);
			}
            for (k = 0; k < cols; k++) {
				if (analysis) {
					printf("A%d%d * -A%d%d/A%d%d + A%d%d = %lf * -(%lf/%lf) + %lf",
					i+1, k+1, j+1, lead_col+1, i+1, lead_col+1, j+1, k+1,
					(*lower)[i][k], main_el_after, first, (*lower)[j][k]);
				}
                (*lower)[j][k] += (*lower)[i][k]*second;
                if (fabs((*lower)[j][k]) < EPS) {
					(*lower)[j][k] = 0.0;
                }
                if (analysis) {
					printf(" = %lf\n", (*lower)[j][k]);
				}
            }
            if (analysis) {
				printf("\nПолучится матрица:\n");
				print_numbered_matrix(lower, unique_numbers, cols+1, rows, 0, file);
				printf("\n");
			}
        }

        i--;
        lead_col--;
    }
}

// вывод в консоль нижнетреугольного вида матрицы
void print_lower_triangular_type(double ***matrix, int **unique_numbers,
								 int cols, int rows, bool analysis) {
	double **lower = NULL;
	char **text = NULL;
	int width;
	int i, j, R;
	
	if (matrix == NULL || unique_numbers == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	if (cols != rows) {
		printf("Основная матрица не квадратная\n");
		return;
	}
	
	// нахождение нижнетреугольново вида
	lower_triangular_type(matrix, unique_numbers, &lower, rows, cols, analysis, NULL);
	
	if (lower != NULL) {	
		// вывод матрицы в консоль
		print_matrix(&lower, cols, rows, &text, &width);
		if (text != NULL) {
			if (analysis) {
				printf("Нижнетреугольный вид матрицы:\n");
			}
			
			for (i = 0; i < rows; i++) {
				for (j = 0; j < width; j++) {
					printf("%c" , text[i][j]);
				}
				printf("\n");
			}
			
			for (i = 0; i < rows; i++) {
				free(lower[i]);
			}
			free(lower);
		
			for (i = 0; i < rows; i++) {
				free(text[i]);
			}
			free(text);
		}
	}
}

////////// Функции, транспонирующие основную матрицу (13) //////////
// нахождение транспонированной матрицы
void transpose(double ***matrix, double ***T, int cols, int rows) {
	int i, j;
	double *row = NULL;
	
	if (matrix == NULL || T == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	// выделение памяти для T
	*T = calloc(rows, sizeof(double*));
	if (*T == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < rows; i++) {
			row = calloc(cols, sizeof(double));
			if (row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free((*T)[j]);
				}
				free(*T);
				*T = NULL;
				return;
			} else {
				(*T)[i] = row;
			}
		}
	}
	
	// транспонирование
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			(*T)[i][j] = (*matrix)[j][i];
		}
	}
}

// вывод транспонированной матрицы
void print_transpose(double ***matrix, int cols, int rows) {
	double **T = NULL;
	int i;
	
	if (matrix == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	if (cols != rows) {
		printf("Основная матрица не квадратная\n");
		return;
	}
	
	transpose(matrix, &T, cols, rows);
	if (T != NULL) {
		// вывод исходной матрицы
		printf("Исходная матрица:\n");
		print_main_matrix(matrix, cols, rows);
		
		printf("\n");
		
		// вывод транспонированной матрицы
		printf("Транспонированная матрица:\n");
		print_main_matrix(&T, cols, rows);
		
		// освобождения памяти у T
		for (i = 0; i < rows; i++) {
			free(T[i]);
		}
		free(T);
	}
}

////////// Функции, ищущие минор (15) //////////
// поиск минора
void search_minor(double ***matrix, double ***minor, int order, int row, int col) {
	int i, k, q, l;
	double *new_row = NULL;
	
	if (matrix == NULL || minor == NULL) {
		return;
	}
	
	*minor = calloc(order, sizeof(double*));
	if (*minor == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < order; i++) {
			new_row = calloc(order, sizeof(double));
			if (new_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (k = 0; k < i; k++) {
					free((*minor)[k]);
				}
				free(*minor);
				return;
			} else {
				(*minor)[i] = new_row;
			}
		}
	}
	
	if (row >= order) {
		row = order-1;
	}
	
	for (i = 0, q = -1; i < order; i++) {
		if (i != row) {
			q++;
		}
		for (k = 0, l = -1; i != row && k < order; k++) {
			if (k != col) {
				l++;
			}
			if (k != col) {
				(*minor)[q][l] = (*matrix)[i][k];
			}
		}
	}
}

// вывод минора в консоль
void print_minor(double ***matrix, int rows, int cols) {
	if (matrix == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	int row, col, i, j;
	char end;
	double **minor = NULL;;
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	printf("Исходная матрица: \n");
	print_main_matrix(matrix, cols, rows);
	
	if (rows != cols) {
		printf("Основная матрица не квадратная\n");
		return;
	}
	
	printf("Введите через пробел номера строки и столбца, "
		   "на пересечении которых находится элемент: ");
	if (scanf("%d%d%c", &row, &col, &end) != 3 || row <= 0 || row > rows ||
		col <= 0 || col > cols || end != '\n') {
		printf("Данные введены некорректно\n");
		return;
	}
	
	// поиск минора
	search_minor(matrix, &minor, rows, row-1, col-1);
	
	// вывод минора
	if (minor != NULL) {
		printf("\nМинор: \n");
		print_main_matrix(&minor, cols-1, rows-1);
		
		for (i = 0; i < rows; i++) {
			free(minor[i]);
		}
		free(minor);
	}
}

////////// Функции, ищущие определитель (14) //////////
// поиск определителя через верхнетреугольный вид матрицы
double search_det_through_step_type(double ***matrix,
	double **equalities, int **unique_numbers,
	int order, bool analysis, int *error, FILE *file) {
	
	double **step = NULL;
	int i, j;
	double det;
	
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL) {
		*error = 1;
		return 0.0;
	}
	
	// приведение побочной матрицы к ступенчатому виду
	step_type(matrix, equalities, unique_numbers, &step, order, order, analysis, file);
	
	// нахождение определителя
	if (analysis) {
		for (i = 0; i < order; i++) {
			if (i == order-1) {
				fprintf(file, "A%d%d = ", i+1, i+1);
			} else {
				fprintf(file, "A%d%d*", i+1, i+1);
			}
		}
	}
	for (i = 0, det = 1; i < order; i++) {
		if (fabs(step[i][i]) < EPS) {
			step[i][i] = 0;
		}
		if (analysis) {
			if (i == order-1) {
				fprintf(file, "(%lf) = ", step[i][i]);
			} else {
				fprintf(file, "(%lf)*", step[i][i]);
			}
		}
		det *= step[i][i];
	}
	
	if (analysis) {
		fprintf(file, "%lf\n", det);
	}
	
	for (i = 0; i < order; i++) {
		free(step[i]);
	}
	free(step);
	
	return det;
}

double search_det_through_lower_type(double ***matrix, int **unique_numbers,
								     int order, int *error) {
	double **lower = NULL;
	int i, j;
	double det;
	
	if (matrix == NULL || unique_numbers == NULL) {
		*error = 1;
		return 0.0;
	}
	
	// приведение побочной матрицы к ступенчатому виду
	lower_triangular_type(matrix, unique_numbers, &lower, order, order, 1, NULL);
	
	printf("det(A) = ");
	for (i = 0; i < order; i++) {
		if (i == order-1) {
			printf("A%d%d = ", i+1, i+1);
		} else {
			printf("A%d%d*", i+1, i+1);
		}
	}
	
	// нахождение определителя
	for (i = 0, det = 1; i < order; i++) {
		if (fabs(lower[i][i]) < EPS) {
			lower[i][i] = 0;
		}
		if (i == order-1) {
			printf("(%lf) = ", lower[i][i]);
		} else {
			printf("(%lf)*", lower[i][i]);
		}
		det *= lower[i][i];
	}
	
	printf("%lf\n", det);
	
	for (i = 0; i < order; i++) {
		free(lower[i]);
	}
	free(lower);
	
	return det;
}

// нахождение определителя для матрицы 2-го порядка
double second_order(double ***matrix, int order, bool analysis, int *error, FILE *file) {
	double det;
	int i;
	double numbers[4] = {(*matrix)[0][0], (*matrix)[1][1],
						 (*matrix)[0][1], (*matrix)[1][0]};
	
	if (matrix == NULL) {
		*error = 1;
		return;
	}
	
	det = (*matrix)[0][0]*(*matrix)[1][1] - (*matrix)[0][1]*(*matrix)[1][0];
	if (analysis) {
		fprintf(file, "A11*A22 - A01*A10 = ");
		for (i = 0; i < 4; i++) {
			if (i % 2 != 0) {
				printf("*");
			} else {
				if (i != 0) {
					printf(" - ");
				}
			}
			if (numbers[i] < 0) {
				fprintf(file, "(%lf)", numbers[i]);
			} else {
				fprintf(file, "%lf", numbers[i]);
			}
			if (i == 3) {
				fprintf(file, " = ");
			}
		}
		fprintf(file, "%lf\n", det);
	}
	
	return det;
}

// нахождение определителя для матрицы 3-го порядка (правило треугольника)
double rule_of_triangle(double ***matrix, int order, int *error, FILE *file) {
	double det;
	int i;
	double numbers[18] = {(*matrix)[0][0], (*matrix)[1][1], (*matrix)[2][2],
						  (*matrix)[0][2], (*matrix)[1][0], (*matrix)[2][1],
						  (*matrix)[2][0], (*matrix)[0][1], (*matrix)[1][2],
						  (*matrix)[0][2], (*matrix)[1][1], (*matrix)[2][0],
						  (*matrix)[0][0], (*matrix)[2][1], (*matrix)[1][2],
						  (*matrix)[2][2], (*matrix)[0][1], (*matrix)[1][0]};
	
	if (matrix == NULL) {
		*error = 1;
		return;
	}
	
	det = (*matrix)[0][0]*(*matrix)[1][1]*(*matrix)[2][2] +
		  (*matrix)[0][2]*(*matrix)[1][0]*(*matrix)[2][1] +
		  (*matrix)[2][0]*(*matrix)[0][1]*(*matrix)[1][2] -
		  (*matrix)[0][2]*(*matrix)[1][1]*(*matrix)[2][0] -
		  (*matrix)[0][0]*(*matrix)[2][1]*(*matrix)[1][2] -
		  (*matrix)[2][2]*(*matrix)[0][1]*(*matrix)[1][0];
		  
	fprintf(file, "A00*A11*A22 + A02*A10*A21 + A20*A01*A12 - ");
	fprintf(file, "A02*A11*A20 - A00*A21*A12 - A22*A01*A10 = ");
	for (i = 0; i < 18; i++) {
		if (i % 3 != 0) {
			fprintf(file, "*");
		} else {
			if (i != 0) {
				if (i < 9) {
					fprintf(file, " + ");
				} else {
					fprintf(file, " - ");
				}
			}
		}
		if (numbers[i] < 0) {
			fprintf(file, "(%lf)", numbers[i]);
		} else {
			fprintf(file, "%lf", numbers[i]);
		}
		if (i == 17) {
			fprintf(file, " = ");
		}
	}
	fprintf(file, "%lf\n", det);
	
	return det;
}

// дорбавление матрицы в текст
void add_matrix_into_text(double ***matrix, int order, char edge, char ***text, int *width_of_level,
						  int height_of_level, int *error) {
	char **copy_text = NULL;
	char *new_row = NULL;
	
	char **text_matrix = NULL;
	char c;
	int width_text_matrix;
	
	int i, j;
	
	if (matrix == NULL || text == NULL || width_of_level == NULL) {
		*error = 1;
		return;
	}
	
	print_matrix(matrix, order, order, &text_matrix, &width_text_matrix);
	
	if (text_matrix == NULL) {
		*error = 2;
		return;
	}
	
	// выравниваем границы
	width_text_matrix++;
	for (i = 0; i < order; i++) {
		new_row = realloc(text_matrix[i], sizeof(char) * width_text_matrix);
		if (new_row == NULL) {
			*error = 2;
			for (j = 0; j < width_text_matrix; j++) {
				free(text_matrix[j]);
			}
			free(text_matrix);
			return;
		} else {
			text_matrix[i] = new_row;
			
			text_matrix[i][width_text_matrix-2] = edge;
			text_matrix[i][width_text_matrix-1] = edge;
			
			for (j = width_text_matrix-1; j > 0; j--) {
				c = text_matrix[i][j];
				text_matrix[i][j] = text_matrix[i][j-1];
				text_matrix[i][j-1] = c;
			}
		}
	}
	
	// копирум матрицу
	copy_text = calloc(height_of_level, sizeof(char*));
	if (copy_text == NULL) {
		*error = 2;
		return;
	} else {
		for (i = 0; i < height_of_level; i++) {
			new_row = calloc((*width_of_level) + width_text_matrix, sizeof(char));
			if (new_row == NULL) {
				*error = 2;
				
				for (j = 0; j < i; j++) {
					free(copy_text[j]);
				}
				free(copy_text);
				
				for (j = 0; j < width_text_matrix; j++) {
					free(text_matrix[j]);
				}
				free(text_matrix);
				
				return;
			} else {
				copy_text[i] = new_row;
				
				for (j = 0; j < *width_of_level; j++) {
					copy_text[i][j] = (*text)[i][j];
				}
			}
		}
	}
	
	// добавляем в конец текста матрицу
	for (i = 0; i < height_of_level; i++) {
		for (j = 0; j < width_text_matrix; j++) {
			if (i < order) {
				copy_text[i][(*width_of_level) + j] = text_matrix[i][j];
			} else {
				copy_text[i][(*width_of_level) + j] = ' ';
			}
		}
	}
	
	//очищаем использованные и старые массивы
	for (i = 0; i < order; i++) {
		free(text_matrix[i]);
	}
	free(text_matrix);
	
	for (i = 0; i < height_of_level; i++) {
		free((*text)[i]);
	}
	free(*text);
	
	// заменяем старый данные новыми
	*text = copy_text;
	*width_of_level = *width_of_level + width_text_matrix;
}

// добавление числа в текст
void add_number_into_text(double number, bool is_int, char ***text, int *width_of_level,
						  int height_of_level, int *error) {
	char **copy_text = NULL;
	char *new_row = NULL;
	
	char *str_number = NULL;
	int num, size_of_number;
	
	int i, j;
	
	if (text == NULL || width_of_level == NULL) {
		*error = 2;
		return;
	}
	
	if (is_int) {
		size_of_number = 0;
	} else {
		size_of_number = 7;
	}
	
	num = (int) number;
	if (num < 0) {
		if (is_int) {
			num = abs(num);
		} else {
			num = fabs(num);
		}
		size_of_number++;
	}
	do {
		num /= 10;
		size_of_number++;
	} while (num != 0);
	
	// копирум матрицу
	copy_text = calloc(height_of_level, sizeof(char*));
	if (copy_text == NULL) {
		*error = 2;
		return;
	} else {
		for (i = 0; i < height_of_level; i++) {
			new_row = calloc((*width_of_level)+size_of_number, sizeof(char));
			if (new_row == NULL) {
				*error = 2;
				for (j = 0; j < i; j++) {
					free(copy_text[j]);
				}
				free(copy_text);
				return;
			} else {
				copy_text[i] = new_row;
				
				for (j = 0; j < *width_of_level; j++) {
					copy_text[i][j] = (*text)[i][j];
				}
			}
		}
	}
	
	// переводим число в строку
	str_number = calloc(size_of_number+1, sizeof(char));
	if (str_number == NULL) {
		*error = 2;
		for (i = 0; i < height_of_level; i++) {
			free(copy_text[i]);
		}
		free(copy_text);
		return;
	} else {
		if (is_int) {
			snprintf(str_number, size_of_number+1,"%d", (int) number);
		} else {
			snprintf(str_number, size_of_number+1,"%lf", number);
		}
		
		for (i = 0; i < size_of_number; i++) {
			copy_text[0][(*width_of_level) + i] = str_number[i];
			for (j = 1; j < height_of_level; j++) {
				copy_text[j][(*width_of_level) + i] = ' ';
			}
		}
	}
	
	//очищаем старый массив
	for (i = 0; i < height_of_level; i++) {
		free((*text)[i]);
	}
	free(*text);
	free(str_number);
	
	// заменяем старый данные новыми
	*text = copy_text;
	*width_of_level = *width_of_level + size_of_number;
}

// добавление символа в текст
void add_char_into_text(char ch, char ***text, int *width_of_level,
						int height_of_level, int *error) {
	char **copy_text = NULL;
	char *new_row = NULL;
	int i, j;
	
	if (text == NULL || width_of_level == NULL) {
		*error = 1;
		return;
	}
	
	// копирум матрицу
	copy_text = calloc(height_of_level, sizeof(char*));
	if (copy_text == NULL) {
		*error = 2;
		return;
	} else {
		for (i = 0; i < height_of_level; i++) {
			new_row = calloc((*width_of_level)+1, sizeof(char));
			if (new_row == NULL) {
				*error = 2;
				for (j = 0; j < i; j++) {
					free(copy_text[j]);
				}
				free(copy_text);
				return;
			} else {
				copy_text[i] = new_row;
				
				for (j = 0; j < *width_of_level; j++) {
					copy_text[i][j] = (*text)[i][j];
				}
			}
		}
	}
	
	// добавляем в конец первой строки символ из переменной ch
	copy_text[0][*width_of_level] = ch;
	
	// запоняем под первой строкой пустые поля символом ' '
	for (i = 1; i < height_of_level; i++) {
		copy_text[i][*width_of_level] = ' ';
	}
	
	// очищаем старый массив
	for (i = 0; i < height_of_level; i++) {
		free((*text)[i]);
	}
	free(*text);
	
	// заменяем старый данные новыми
	*text = copy_text;
	*width_of_level = *width_of_level + 1;
}

// вывод текстовых блоков с решением
void print_text_levels(char ****text, int **width_of_levels, int rows, int *error, FILE *file) {
	int i, j, k;
	
	if (text == NULL || width_of_levels == NULL) {
		*error = 1;
		return;
	}
	
	if (text == NULL || width_of_levels == NULL) {
		*error = 2;
		return;
	}
	
	for (i = 0; i < rows; i++) {
		for (j = 0; j < rows; j++) {
			for (k = 0; k < ((j != 0 && j != rows-1) ? (*width_of_levels)[j]-3 : (*width_of_levels)[j]); k++) {
				fprintf(file, "%c", (*text)[j][i][k]);
			}
			if (j != rows-1 && i == 0) {
				fprintf(file, " = ");
			} else {
				fprintf(file, "   ");
			}
		}
		fprintf(file, "\n");
	}
}

void print_decision(char ***level, int *width_of_level, int height_of_level,
					double ***matrix, double ***minor, int order, int row, int i, int *error) {
	if (level == NULL || width_of_level == NULL || minor == NULL) {
		*error = 1;
		return;
	}
	
	add_number_into_text((*matrix)[row][i], 0, level, width_of_level, height_of_level, error);
	add_char_into_text('*', level, width_of_level, height_of_level, error);
	add_char_into_text('(', level, width_of_level, height_of_level, error);
	add_number_into_text(-1, 1, level, width_of_level, height_of_level, error);
	add_char_into_text(')', level, width_of_level, height_of_level, error);
	add_char_into_text('^', level, width_of_level, height_of_level, error);
	add_char_into_text('(', level, width_of_level, height_of_level, error);	
	add_number_into_text(row+1, 1, level, width_of_level, height_of_level, error);
	add_char_into_text('+', level, width_of_level, height_of_level, error);
	add_number_into_text(i+1, 1, level, width_of_level, height_of_level, error);
	add_char_into_text(')', level, width_of_level, height_of_level, error);
	add_char_into_text('*', level, width_of_level, height_of_level, error);
	add_matrix_into_text(minor, order-1, '|', level, width_of_level, height_of_level, error);
	add_char_into_text(' ', level, width_of_level, height_of_level, error);
	add_char_into_text('+', level, width_of_level, height_of_level, error);
	add_char_into_text(' ', level, width_of_level, height_of_level, error);
}

// разложение по строке
double decomposition_det_row(double ***matrix, int order, int row, int level,
							 char ****text, int **width_of_levels, int *error) {
	int i, j;
	double result;
	double **minor = NULL;
	
	if (*error) {
		return 0.0;
	}
	
	if (order == 2) {
		return second_order(matrix, order, 0, error, NULL);
	}
	
	if ((*text)[level] == NULL) {
		(*text)[level] = calloc(order+level-1, sizeof(char*));
		if ((*text)[level] == NULL) {
			*error = 2;
			return 0;
		}
	}
	
	if (row >= order) {
		row = order-1;
	}
	
	for (i = 0, result = 0; i < order; i++) {
		search_minor(matrix, &minor, order, row, i);
		
		if (minor == NULL) {
			*error = 1;
			return 0;
		}
		
		result += (*matrix)[row][i] * pow(-1, row+1 + i+1) *
				  decomposition_det_row(&minor, order-1, row, level+1, text, width_of_levels, error);
		
		print_decision(&((*text)[level]), &((*width_of_levels)[level]), order+level-1, matrix, &minor, order, row, i, error);
		
		for (j = 0; j < order; j++) {
			free(minor[j]);
		}
		free(minor);
	}
	
	return result;
}

// разложение по столбцу
double decomposition_det_column(double ***matrix, int order, int column, int level,
							    char ****text, int **width_of_levels, int *error) {
	int i, j;
	double result;
	double **minor = NULL;
	
	if (*error) {
		return 0.0;
	}
	
	if (order == 2) {
		return second_order(matrix, order, 0, error, NULL);
	}
	
	if ((*text)[level] == NULL) {
		(*text)[level] = calloc(order+level-1, sizeof(char*));
		if ((*text)[level] == NULL) {
			*error = 2;
			return 0;
		}
	}
	
	if (column >= order) {
		column = order-1;
	}
	
	for (i = 0, result = 0; i < order; i++) {
		search_minor(matrix, &minor, order, i, column);
		
		if (minor == NULL) {
			*error = 1;
			return 0;
		}
		
		result += (*matrix)[i][column] * pow(-1, i+1 + column+1) *
				  decomposition_det_column(&minor, order-1, column, level+1, text, width_of_levels, error);
		
		print_decision(&((*text)[level]), &((*width_of_levels)[level]), order+level-1, matrix, &minor, order, i, column, error);
		
		for (j = 0; j < order; j++) {
			free(minor[j]);
		}
		free(minor);
	}
	
	return result;
}

// поиск определителя
double determinant(double ***matrix, double **equalities, int **unique_numbers,
				   int rows, int cols, bool analysis,
				   int *error, int *choice_method, int *choice_col_or_row, FILE *file) {
	int key = 1; // выбор метода поиск определителя
	int col_or_row; // выбор строки/столбца для разложения
	char end;
	double det;
	
	char ***text = NULL;
	int *width_of_levels = NULL;
	
	int i, j;
	
	if (matrix == NULL || error == NULL || choice_method == NULL || choice_col_or_row == NULL) {
		*error = 1;
		return;
	}
	
	if (rows == 0) {
		*error = 4;
		return;
	}
	
	if (rows != cols) {
		*error = 5;
		return 0.0;
	}
	
	if (cols < 1) {
		*error = 6;
		return 0.0;
	}
	
	if (*choice_method != 0) {
		key = *choice_method;
	}
	
	if (analysis && !(*choice_method)) {
		printf("Выберите способ вычисления определителя: \n"
			   "1 - Через верхнетреугольный вид матрицы\n"
			   "2 - Через нижнетреугольный вид матрицы\n");
		   
		if (rows <= 6) {
			printf("3 - Разложение по строке\n"
		   		   "4 - Разложение по столбцу\n");
		}
		
		if (rows <= 3) {
			printf("5 - Правило треугольника\n");
		}
		
		printf("-> ");
		if (scanf("%d%c", &key, &end) != 2 || (key == 5 && rows > 3)
			|| ((key == 3 || key == 4) && rows > 6) || key > 5 || key < 1 || end != '\n') {
			*error = 3;
			return 0.0;
		}
	}
	
	switch (key) {
		case 1:
			det = search_det_through_step_type(matrix, equalities, unique_numbers,
											   rows, analysis, error, file);
			break;
		case 2:
			det = search_det_through_lower_type(matrix, unique_numbers,
												rows, error);
			break;	
	}
	
	if (key == 5) {
		switch (rows) {
			case 2:
				det = second_order(matrix, rows, 1, error, file);
			case 3:
				det = rule_of_triangle(matrix, rows, error, file);
		}
	}
	
	if (rows <= 6 && (key == 3 || key == 4)) {
		// ввод номера строки/столбца
		if (!(*choice_col_or_row)) {
			if (key == 3) {
				printf("Введите номер строки: ");
			} else {
				printf("Введите номер столбца: ");
			}
			if (scanf("%d%c", &col_or_row, &end) != 2 || col_or_row < 1 || col_or_row > cols || end != '\n') {
				*error = 3;
				return 0.0;
			}
			*choice_col_or_row = col_or_row;
		}
		
		// создание первого и последнего уровней
		text = calloc(rows, sizeof(char**));
		if (text == NULL) {
			*error = 2;
			return 0.0;
		} else {
			text[0] = calloc(rows, sizeof(char*));
			if (text[0] == NULL) {
				*error = 2;
				free(text);
				return 0.0;
			}
				
			text[rows-1] = calloc(rows, sizeof(char*));
			if (text[0] == NULL) {
				*error = 2;
				free(text[0]);
				free(text);
				return 0.0;
			}
		}
		
		// выделение памяти для массива с размерами (шириной) блоков
		width_of_levels = calloc(rows, sizeof(int));
		if (width_of_levels == NULL) {
			free(text[0]);
			free(text[rows-1]);
			free(text);
			*error = 2;
			return 0.0;
		}
			
		// добавляем исходную матрицы на нулевой уровень (закулисья кхм.. кхм... )
		add_matrix_into_text(matrix, rows, '|', &(text[0]),
						 	 &(width_of_levels[0]), rows, error);
			
		// находим определитель
		switch (key) {
			case 3:
				det = decomposition_det_row(matrix, rows, col_or_row-1, 1,
											&text, &width_of_levels, error);
				break;
			case 4:
				det = decomposition_det_column(matrix, rows, col_or_row-1, 1,
											   &text, &width_of_levels, error);
				break;
		}
			
		// записываем на последний уровень сам определитель
		add_number_into_text(det, 0, &(text[rows-1]), &(width_of_levels[rows-1]), rows, error);
		
		// запись решения
		print_text_levels(&text, &width_of_levels, rows, error, file);
		
		// очистка памяти
		for (i = 0; i < rows; i++) {
			for (j = 0; j < rows; j++) {
				free(text[i][j]);
			}
			free(text[i]);
		}
	
		free(text);
		free(width_of_levels);
	}
	
	*choice_method = key;
	
	return det;
}

void determinant_error(int *error) {
	switch (*error) {
		case 1:
			printf("Критическая ошибка\n");
			break;
		case 2:
			printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
			break;
		case 3:
			printf("Ошибка ввода данных\n");
			break;
		case 4:
			printf("Уравнений в системе нет\n");
			break;
		case 5:
			printf("Основная матрица не квадратная\n");
			break;
		case 6:
			printf("Определитель матрицы слишком мал\n");
			break;
	}
}

// вывод определителя в консоль
void print_determinant(double ***matrix, double **equalities, int **unique_numbers,
					   int rows, int cols, bool analysis) {
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL) {
		printf("Критическая ошибка\n");
		return;
	}
	
	int error = 0;
	int choice_method = 0;
	int choice_col_or_row = 0;
	double det;
	
	det = determinant(matrix, equalities, unique_numbers, rows, cols,
					  analysis, &error, &choice_method, &choice_col_or_row, stdout);
	
	determinant_error(&error);
	
	if (error) {
		printf("Определитель: %lf\n", det);
	}
}

////////// Функции, ищущие алгебраическое дополнение (16) //////////
// поиск алгебраического дополнения
double algebraic_addition(double ***matrix, int order, int row, int col,
						  bool analysis, int *error, int *choice_method, int *choice_col_or_row,
						  double **equalities, int **unique_numbers) {
	double **minor = NULL;
	int i;
	double det;
	double alg;
	
	if (matrix == NULL || choice_method == NULL || choice_col_or_row == NULL
		|| equalities == NULL || unique_numbers == NULL) {
		*error = 1;
		return 0.0;
	}
	
	search_minor(matrix, &minor, order, row, col);
	
	det = 0;
	if (analysis) {
		printf("Минор элемента A%d%d:\n", row+1, col+1);
		det = determinant(&minor, equalities, unique_numbers, order-1, order-1, analysis,
						  error, choice_method, choice_col_or_row, stdout);
		printf("\n");
		
		if (*error) {
			return 0.0;
		}
		
	} else {
		det = search_det_through_step_type(&minor, equalities, unique_numbers, order-1, 0, error, stdout);
	}
	
	for (i = 0; i < order-1; i++) {
		free(minor[i]);
	}
	free(minor);
	
	alg = pow(-1, row+1 + col+1) * det;
	
	if (analysis) {
		printf("A%d%d = (-1)^(%d + %d)*(%lf) = (-1)^(%d)*(%lf) = %lf\n",
			   row+1, col+1, row+1, col+1, det, row+1+col+1, det, alg);
	}
	
	return alg;
}

// вывод алгебраического дополнения
void print_algebraic_addition(double ***matrix, double **equalities, int **unique_numbers,
							  int rows, int cols, bool analysis) {
	if (matrix == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	printf("Исходная матрица:\n");
	print_main_matrix(matrix, cols, rows);
	printf("\n");
	
	if (rows != cols) {
		printf("Основная матрица не квадратная\n");
		return;
	}
	
	int row, col;
	char end;
	double alg;
	
	int error = 0;
	int choice_method = 0;
	int choice_col_or_row = 0;
	
	printf("Введите через пробел номера строки и столбца, "
		   "на пересечении которых находится элемент: ");
	if (scanf("%d%d%c", &row, &col, &end) != 3 || row <= 0 || row > rows ||
		col <= 0 || col > cols || end != '\n') {
		printf("Данные введены некорректно\n");
		return;
	}
	
	printf("\n");
	alg = algebraic_addition(matrix, rows, row-1, col-1, analysis,
							 &error, &choice_method, &choice_col_or_row,
							 equalities, unique_numbers);
	
	determinant_error(&error);
	if (error) {
		if (fabs(alg) < EPS) {
			alg = 0;
		}
		printf("Алгебраическое дополнение: %lf\n", alg);
	}
}

////////// Функции, находящие обратную матрицу (17) //////////
void invertible_matrix(double ***matrix, double **equalities,
					   int **unique_numbers, double ***invertible,
					   int rows, int cols, bool analysis) {
	double alg;
	double **T = NULL;
	double det;
	double *new_row = NULL;
	
	int error = 0;
	int choice = 0;
	int decomposition = 0;
	int i, j;
	
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL || invertible == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows == 0) {
		printf("Уравнений в системе нет\n");
		return;
	}
	
	if (rows != cols) {
		printf("Основная матрица не квадратная\n");
		return;
	}
	
	*invertible = calloc(rows, sizeof(double*));
	if (*invertible == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < rows; i++) {
			new_row = calloc(cols, sizeof(double));
			if (new_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				for (j = 0; j < i; j++) {
					free((*invertible)[j]);
				}
				free(*invertible);
				*invertible = NULL;
				return;
			} else {
				(*invertible)[i] = new_row;
			}
		}
	}
	
	T = calloc(rows, sizeof(double*));
	if (T == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		for (j = 0; j < rows; j++) {
			free((*invertible)[j]);
		}
		free(*invertible);
		*invertible = NULL;
		return;
	} else {
		for (i = 0; i < rows; i++) {
			new_row = calloc(cols, sizeof(double));
			if (new_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				
				for (j = 0; j < rows; j++) {
					free((*invertible)[j]);
				}
				free(*invertible);
				*invertible = NULL;
				
				for (j = 0; j < i; j++) {
					free(T[j]);
				}
				free(T);
				
				return;
			} else {
				T[i] = new_row;
			}
		}
	}
	
	// поиск определителя основной матрицы
	det = determinant(matrix, equalities, unique_numbers, rows, cols,
					  analysis, &error, &choice, &decomposition, stdout);
	if (error) {
		return;
	}
	
	if (det == 0) {
		printf("Обратной матрицы нет, так как определитель равен нулю\n");
		return;
	}
	
	if (analysis) {
		printf("\nОпределитель основной матрицы: %lf\n", det);
		printf("\n");
	}
	
	// создание матрицы алг. дополнений и деление каждого элемента нп аопределитель
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			alg = algebraic_addition(matrix, rows, i, j, analysis,
								  	 &error, &choice, &decomposition,
									 equalities, unique_numbers);
			(*invertible)[i][j] = alg;
		}
	}
	
	// вывод матриц алгебраических элементов дополнений
	if (analysis) {
		printf("Матрица алгебраических дополнений:\n");
		print_main_matrix(invertible, cols, rows);
	}
	
	// заполнение промежуточной матрицы
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			T[i][j] = (*invertible)[i][j];
		}
	}
	
	// транспонирование матрицы (получение присоединённой)
	transpose(&T, invertible, cols, rows);
	
	// вывод присоединённой матрицы
	if (analysis) {
		printf("\nПрисоединённая матрица:\n");
		print_main_matrix(invertible, cols, rows);
		printf("\n");
	}
	
	// деление всех элементов матрицы на определитель
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			(*invertible)[i][j] = (*invertible)[i][j]/det;
		}
	}
	
	for (j = 0; j < rows; j++) {
		free(T[j]);
	}
	free(T);
}

// вывод обратной матрицы
void print_invertible_matrix(double ***matrix, double **equalities,
							 int **unique_numbers, int rows, int cols,
							 bool analysis) {
	double **invertible = NULL;
	int i, j;
	
	if (matrix == NULL || equalities == NULL || unique_numbers == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	invertible_matrix(matrix, equalities, unique_numbers, &invertible, rows, cols, analysis);
	
	// вывод обратной матрицы
	if (invertible != NULL) {
		printf("Обратная матрица:\n");
		print_main_matrix(&invertible, cols, rows);
		
		for (i = 0; i < rows; i++) {
			free(invertible[i]);
		}
		free(invertible);
	}
}

////////// Функции, решающие СЛАУ (19) //////////
// метод Крамера
void kramers_method(double ***matrix, int **unique_numbers,
			 		double **equalities, int order, bool analysis, FILE *file) {
	double **var_matrix = NULL; // матрица, получающаяся заменой столбца на свободные чллены
	double *new_row = NULL;
	double *var_dets = NULL; // определители неизвестных
	double det, var_det;
	
	int error = 0;
	int choice_method = 0;
	int choice_col_or_row = 0;
	double result;
	
	int i, j, k;
	
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	// вывод исходной системы
	fprintf(file, "Исходная система:\n");
	print_console_all(matrix, unique_numbers, equalities, order, order, file);
	fprintf(file, "\n");
	
	
	// нахождение определителя основной матрицы
	fprintf(file, "Поиск определителя основной матрицы:\n");
	det = determinant(matrix, equalities, unique_numbers, order, order, analysis,
					  &error, &choice_method, &choice_col_or_row, file);
	determinant_error(&error);
	if (error) {
		return;
	}
	fprintf(file, "\nОпределитель основной матрицы: %lf\n", det);
	fprintf(file, "det(A) = %lf\n", det);
	
	// вывод ответа, если определитель равен 0
	if (det == 0) {
		fprintf(file, "Ответ: решений нет или их бесконечно много");
		return;
	}
	
	// нахождение определителей неизвестных
	var_matrix = calloc(order, sizeof(double));
	if (var_matrix == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	} else {
		for (i = 0; i < order; i++) {
			new_row = calloc(order, sizeof(double));
			
			if (new_row == NULL) {
				printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
				
				for (j = 0; j < i; j++) {
					free(var_matrix[j]);
				}
				free(var_matrix);
				
				return;
			} else {
				var_matrix[i] = new_row;
			}
		}
	}
	
	var_dets = calloc(order, sizeof(double));
	if (var_dets == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		
		for (j = 0; j < order; j++) {
			free(var_matrix[j]);
		}
		free(var_matrix);
		
		return;
	}
	
	fprintf(file, "\nПоиск определителей неизвестных: \n\n");
	for (k = 0; k < order; k++) {
		for (i = 0; i < order; i++) {
			for (j = 0; j < order; j++) {
				if (j == k) {
					var_matrix[i][j] = (*equalities)[i];
					continue;
				}
				var_matrix[i][j] = (*matrix)[i][j];
			}
		}
		
		fprintf(file, "Определитель неизвестной x%d:\n", (*unique_numbers)[k]);
		var_det = determinant(&var_matrix, equalities, unique_numbers, order, order,
							  analysis, &error, &choice_method, &choice_col_or_row, file);
		determinant_error(&error);
		if (error) {
			for (i = 0; i < order; i++) {
				free(var_matrix[i]);
			}
			free(var_matrix);
			free(var_dets);
			return;
		}
		
		fprintf(file, "det(x%d) = %lf\n\n", (*unique_numbers)[k], var_det);
		var_dets[k] = var_det;
	}
	
	// вывод решения
	fprintf(file, "Ответ: \n");
	for (i = 0, result = 0; i < order; i++) {
		result = var_dets[i]/det;
		if (fabs(result) < EPS) {
			fprintf(file, "x%d = det(x%d)/det(A) = (%lf)/(%lf) = %lf\n", (*unique_numbers)[i], (*unique_numbers)[i], var_dets[i], det, 0);
		} else {
			fprintf(file, "x%d = det(x%d)/det(A) = (%lf)/(%lf) = %lf\n", (*unique_numbers)[i], (*unique_numbers)[i], var_dets[i], det, result);
		}
	}
	
	for (i = 0; i < order; i++) {
		free(var_matrix[i]);
	}
	free(var_matrix);
	free(var_dets);
}

void prod(double ***matrix, double **equalities, double **roots,
		  int order, int **unique_numbers, bool analysis, FILE *file) {
	int i, j;
	
	if (matrix == NULL || equalities == NULL || roots == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	for (i = 0; i < order; i++) {
		(*roots)[i] = 0;
		if (analysis) {
			printf("%d = ", *unique_numbers[i]);
		}
		for (j = 0; j < order; j++) {
			printf("!");
			(*roots)[i] += (*matrix)[i][j] * (*equalities)[j];
			if (analysis && 0) {
				if ((*matrix)[i][j] < 0) {
					printf("(%lf) * ", (*matrix)[i][j]);
				} else {
					printf("%lf ", (*matrix)[i][j]);
				}
				if ((*equalities)[j] < 0) {
					printf("(%lf) = ", (*equalities)[j]);
				} else {
					printf("%lf = ", (*equalities)[j]);
				}
				printf("%lf\n", (*roots)[i]);
			}
		}
	}
}

void matrix_method(double ***matrix, int **unique_numbers,
			 	   double **equalities, int cols, int rows, FILE *file) {
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	double **invertible = NULL;
	double *roots = NULL;
	int i;
	double result;
	
	roots = calloc(cols, sizeof(double));
	if (roots == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	}
	
	// нахождение обратной матрицы
	invertible_matrix(matrix, equalities, unique_numbers, &invertible, rows, cols, 0);
	
	if (invertible == NULL) {
		printf("Ответ: решений нет или их бесконечно много\n");
		free(roots);
		return;
	}
	
	// умножение обратной матрицы на вектор-столбец свободных членов
	prod(&invertible, equalities, &roots, rows, unique_numbers, 1, file);
	
	// вывод ответа
	for (i = 0; i < cols; i++) {
		printf("x%d = %lf\n", unique_numbers[i], roots[i]);
	}
	
	for (i = 0; i < rows; i++) {
		free(invertible[i]);
	}
	free(invertible);
	free(roots);
}


/*void type_of_unknown_numbers(double ***matrix, double **roots, int cols,
							 int rows) {
	int i, j, k, countzero;
	
	if (matrix == NULL || roots == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	for (i = 0; i < rows; i++) {
		for (j = i; j < cols+1; j++) {
			for (countzero = 0, k = rows; k >= 0; k--) {
				if ((*matrix)[k][j] == 0) {
					countzero++;
				} else {
					break;
				}
			}
			if ((R-countzero) == i+1) {
				(*roots)[j][1] = 1;
				break;
			}
		}
	}
}*/

void gauss_method(double ***matrix, int **unique_numbers, double **equalities,
				  int rows, int cols, bool analysis, FILE *file) {
	double *roots = NULL;
	double **step = NULL;
	int R, r;
	int i;
	
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	roots = calloc(cols, sizeof(double));
	if (roots == NULL) {
		printf("Ошибка при выделении памяти. Попробуйте повторить действие или перезапустить программу\n");
		return;
	}
	
	// нахождение ступенчатого вида
	step_type(matrix, equalities, unique_numbers, &step, rows, cols, analysis, file);
	
	// нахождение рангов матрицы
	r = rank(&step, rows, cols, analysis, 0);
	if (analysis) {
		printf("Ранг основной матрицы r = %d\n\n", r);
	}
	R = rank(&step, rows, cols, analysis, 1);
	if (analysis) {
		printf("Ранг расширенной матрицы R = %d\n\n", R);
	}
	
	if (r != R) {
		printf("Ранги основной и расширенной матрицы несвопадают, следовательно решений нет\n");
		printf("Ответ: решений нет\n");
		for (i = 0; i < rows; i++) {
			free(step[i]);
		}
		free(step);
		free(roots);
		return;
	}
	
	// определяем типы переменных
	//type_of_unknown_numbers(matrix, &roots, cols, rows);
}

// выбор способа решения СЛАУ
void solving(double ***matrix, int **unique_numbers,
			 double **equalities, int cols, int rows, int analysis) {
	int key;
	char end;
	FILE *file = stdout;
	char *file_name = NULL;
	int error = 0;
	
	if (matrix == NULL || unique_numbers == NULL || equalities == NULL) {
		printf("Критическая ошибка!\n");
		return;
	}
	
	if (rows < 2) {
		printf("СЛАУ имеет менее 2-х уравнений\n");
		return;
	}
	
	printf("Записать решение в файл? (1 - да, 0 - нет): ");
	if (scanf("%d%c", &key, &end) != 2 || key != 1 || end != '\n') {
		key = 0;
	}
	
	if (key == 1) {
		fflush(stdin);
		input_file(&file_name, &error);
		if (error) {
			printf("Ошибка!\n");
		} else {
			file = fopen(file_name, "a");
			if (file == NULL) {
				printf("Файл не удалось открыть\n");
				return;
			}
		}
	}
	
	if (cols == rows && analysis) {
		printf("Выберите вариант решения СЛАУ:\n"
			   "1 - Метод Гаусса\n"
			   "2 - Метод Крамера\n"
			   "3 - Матричный способ\n");
		printf("-> ");
		if (scanf("%d%c", &key, &end) != 2 || key < 1 || key > 3 || end != '\n') {
			printf("Такой операции не существует\n");
			return;
		}
		switch (key) {
			case 1:
				gauss_method(matrix, unique_numbers, equalities, rows, cols, analysis, file);
				break;
			case 2:
				kramers_method(matrix, unique_numbers, equalities, rows, analysis, file);
				break;
			case 3:
				matrix_method(matrix, unique_numbers, equalities, cols, rows, file);
				break;
		}
	} else {
		gauss_method(matrix, unique_numbers, equalities, rows, cols, analysis, file);
	}
	
	if (file != stdout) {
		fclose(file);
	}
}

int main() {
	system("chcp 1251 > nul");
	
	int key, cols = 0, rows = 0;
	bool active = 1, analysis = 0;
	char end;
	
	double **matrix = NULL;
	double *equalities = NULL;
	int *unique_numbers = NULL;
	
	int i;
	
	while(active) {
		fflush(stdin);
		printf("Выберите операцию:\n"
			   "0 - Выход\n"
			   "1 - Добавить уравнение\n"
			   "2 - Удалить уравнение\n"
			   "3 - Удалить все уравнения\n"
			   "4 - Вставить уравнение\n"
			   "5 - Поменять уравнения местами\n"
			   "6 - Добавить уравнения из файла\n"
			   "7 - Вывести одно уравнение\n"
			   "8 - Вывести СЛАУ\n"
			   "9 - Вывести основную матрицу\n"
			   "10 - Вывести расширенную матрицу\n"
			   "11 - Вывести трапецеидального вид матрицы\n"
			   "12 - Вывести нижнетреугольный вид матрицы\n"
			   "13 - Транспонировать основную матрицу\n"
			   "14 - Найти определитель\n"
			   "15 - Найти минор\n"
			   "16 - Найти алгебраическое дополнение\n"
			   "17 - Найти обратную матрицу\n"
			   "18 - Найти ранги матрицы\n"
			   "19 - Решить СЛАУ\n");
		if (analysis) {
			printf("20 - Выключить разбор операций\n");
		} else {
			printf("20 - Включить разбор операций\n");
		}
		printf("-> ");
		if (scanf("%d%c", &key, &end) != 2 || end != '\n') {
			printf("Ошибка ввода данных!\n\n");
			fflush(stdin);
			continue;
		}
		switch (key) {
			case 0:
				close(&active);
				break;
			case 1:
				add_console(&matrix, &unique_numbers, &equalities, &cols, &rows);
				break;
			case 2:
				del(&matrix, &unique_numbers, &equalities, &cols, &rows);
				break;
			case 3:
				del_all(&matrix, &unique_numbers, &equalities, &cols, &rows);
				break;
			case 4:
				insert(&matrix, &unique_numbers, &equalities, &cols, &rows);
				break;
			case 5:
				swap(&matrix, &equalities, cols, rows);
				break;
			case 6:
				loading(&matrix, &unique_numbers, &equalities, &cols, &rows);
				break;
			case 7:
				print_console_once(&matrix, &unique_numbers, &equalities, cols, rows, -1, stdout);
				break;
			case 8:
				print_console_all(&matrix, &unique_numbers, &equalities, cols, rows, stdout);
				break;
			case 9:
				print_main_matrix(&matrix, cols, rows);
				break;
			case 10:
				print_extended_matrix(&matrix, &equalities, &unique_numbers, cols, rows, analysis);
				break;
			case 11:
				print_step_type(&matrix, &equalities, &unique_numbers, cols, rows, analysis);
				break;
			case 12:
				print_lower_triangular_type(&matrix, &unique_numbers, cols, rows, analysis);
				break;
			case 13:
				print_transpose(&matrix, cols, rows);
				break;
			case 14:
				print_determinant(&matrix, &equalities, &unique_numbers, rows, cols, analysis);
				break;
			case 15:
				print_minor(&matrix, rows, cols);
				break;
			case 16:
				print_algebraic_addition(&matrix, &equalities, &unique_numbers, rows, cols, analysis);
				break;
			case 17:
				print_invertible_matrix(&matrix, &equalities, &unique_numbers, rows, cols, analysis);
				break;
			case 18:
				print_rank(&matrix, &equalities, &unique_numbers, cols, rows, analysis);
				break;
			case 19:
				solving(&matrix, &unique_numbers, &equalities, cols, rows, analysis);
				break;
			case 20:
				analysis = !analysis;
				break;
			default:
				printf("Такой операции не существует\n");
				break;
		}
		printf("\n");
	}
	
	for (i = 0; i < rows; i++) {
		free(matrix[i]);
	}
	free(matrix);
	free(equalities);
	free(unique_numbers);
	
	system("pause");
	return 0;
}
