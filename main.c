#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <locale.h>

// Функция для корректного расчета загрузки CPU
double get_cpu_load() {
    static FILETIME prev_idle, prev_kernel, prev_user;
    FILETIME idle, kernel, user;

    if (!GetSystemTimes(&idle, &kernel, &user)) return 0.0;

    ULARGE_INTEGER i, k, u, pi, pk, pu;
    i.LowPart = idle.dwLowDateTime; i.HighPart = idle.dwHighDateTime;
    k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
    u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;

    pi.LowPart = prev_idle.dwLowDateTime; pi.HighPart = prev_idle.dwHighDateTime;
    pk.LowPart = prev_kernel.dwLowDateTime; pk.HighPart = prev_kernel.dwHighDateTime;
    pu.LowPart = prev_user.dwLowDateTime; pu.HighPart = prev_user.dwHighDateTime;

    prev_idle = idle; prev_kernel = kernel; prev_user = user;

    ULONGLONG idle_diff = i.QuadPart - pi.QuadPart;
    ULONGLONG kernel_diff = k.QuadPart - pk.QuadPart;
    ULONGLONG user_diff = u.QuadPart - pu.QuadPart;
    ULONGLONG total_diff = kernel_diff + user_diff;

    if (total_diff == 0) return 0.0;
    return (double)(total_diff - idle_diff) * 100.0 / total_diff;
}

// Функция вывода списка запущенных процессов
void list_processes() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    printf("\nID\t\tИмя процесса\n");
    printf("------------------------------------\n");
    
    if (Process32First(hSnapshot, &pe)) {
        int count = 0;
        do {
            printf("%d\t\t%s\n", pe.th32ProcessID, pe.szExeFile);
            if (++count > 20) break; // Выводим только первые 10 для компактности
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
}

void print_system_info() {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    double cpu = get_cpu_load();

    printf("=== SYSTEM MONITOR ===\n");
    printf("CPU Load: %.2f%%\n", cpu);
    printf("RAM Usage: %llu MB / %llu MB (%d%%)\n",
           (mem.ullTotalPhys - mem.ullAvailPhys) / 1024 / 1024,
           mem.ullTotalPhys / 1024 / 1024,
           mem.dwMemoryLoad);
}

int main() {
    setlocale(LC_ALL, "ru_RU.utf8");
    while (1) {
        system("cls");
        print_system_info();
        list_processes();
        Sleep(1000);
    }
    return 0;
}
