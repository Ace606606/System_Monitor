#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    GtkWidget *label_cpu;
    GtkWidget *label_mem;
} UpdateData;

//Функция для получения загрузки процессора
float get_cpu_usage() {
	FILE *file;
	char buffer[256];
	float cpu_usage = 0.0;

	// Открытик файла /proc/stat для получения данных о CPU
	file = fopen("/proc/stat", "r");
	
	if (file != NULL) {
		fgets(buffer, sizeof(buffer), file);
		//разделяем строку на компоненты
		unsigned long user, nice, system, idle, iowait, irq, softirq;
		sscanf(buffer, "cpu, %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
		fclose(file);

		//расчёт нагрузки процессора
		unsigned long total = user + nice + system + idle + iowait + irq + softirq;
		unsigned long active = user + nice + system + irq +softirq;
		cpu_usage = ((float)active / total) * 100.0;
	}

	return cpu_usage;
}

//функция для получения использования памяти
float get_memory_usage() {
	FILE *file;
	char buffer[256];
	float memory_usage = 0.0;

	// /proc/meminfo
	file = fopen("/proc/meminfo", "r");
	if (file != NULL) {
		unsigned long total_memory, free_memory;
		while (fgets(buffer, sizeof(buffer), file)) {
			if (sscanf(buffer, "MemTotal: %lu KB", &total_memory) == 1) {
				continue;
			}
			if (sscanf(buffer, "MemFree: %lu kB", &free_memory) == 1) {
                break;
            }
		}
		fclose(file);

		//Расчёт использования памяти
		memory_usage = ((float)(total_memory - free_memory) / total_memory) * 100.0;
	}
	return memory_usage;
}

//обновление данных и обновление интерфейса
gboolean update_data(gpointer user_data) {
	UpdateData *data = (UpdateData *)user_data;

	float cpu_usage = get_cpu_usage();
	float memory_usage = get_memory_usage();

	//обнновляем метки с новой информацией
	char cpu_text[256];
	char mem_text[256];
	snprintf(cpu_text, sizeof(cpu_text), "CPU Usage: %.2f%%", cpu_usage);
	snprintf(mem_text, sizeof(mem_text), "Memory Usage: %.2f%%", memory_usage);

	gtk_label_set_text(GTK_LABEL(data->label_cpu), cpu_text);
    gtk_label_set_text(GTK_LABEL(data->label_mem), mem_text);

	return TRUE; //продолжаем обновление
}

int main(int argc, char *argv[]) {
	GtkWidget *window;
    GtkWidget *label_cpu;
    GtkWidget *label_mem;
    GtkWidget *box;

    // Инициализация GTK
    gtk_init(&argc, &argv);

    // Создаем окно
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "System Monitor");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Создаем контейнер для элементов
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Создаем метки для отображения информации
    label_cpu = gtk_label_new("CPU Usage: 0.0%");
    label_mem = gtk_label_new("Memory Usage: 0.0%");
    gtk_box_pack_start(GTK_BOX(box), label_cpu, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), label_mem, TRUE, TRUE, 0);

	// Создаем структуру для передачи данных
	UpdateData *data = g_new(UpdateData, 1);
		data->label_cpu = label_cpu;
		data->label_mem = label_mem;

    // Настроим обновление данных каждую секунду
    g_timeout_add(1000, (GSourceFunc)update_data, data);

    // Показываем окно
    gtk_widget_show_all(window);

    // Запуск главного цикла GTK
    gtk_main();

    return 0;
}


