#include <windows.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <locale.h>
#include <shlwapi.h>  // для PathFindFileNameA (потребуется shlwapi.lib)

using namespace std;

// Функция получения базового имени файла из пути (без использования Shlwapi)
string GetFileNameFromPath(const string& path)
{
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != string::npos)
        return path.substr(lastSlash + 1);
    return path;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Создаём папку files, если её нет
    CreateDirectoryA("files", NULL);

    HANDLE hMailslot = CreateMailslot(
        "\\\\.\\mailslot\\demo_mailslot",
        0,                         // произвольный размер сообщения (до 64KB)
        MAILSLOT_WAIT_FOREVER,
        NULL
    );

    if (hMailslot == INVALID_HANDLE_VALUE)
    {
        cerr << "CreateMailslot failed. Last error: " << GetLastError() << endl;
        cout << "Press any key to exit.";
        cin.get();
        return 1;
    }

    cout << "Mailslot created. Waiting for messages..." << endl;

    const DWORD BUFFER_SIZE = 65536; // 64KB
    char* buffer = new char[BUFFER_SIZE];
    DWORD dwBytesRead;

    while (true)
    {
        if (!ReadFile(
            hMailslot,
            buffer,
            BUFFER_SIZE - 1,   // оставляем место для завершающего нуля? На самом деле для бинарных данных мы не будем добавлять ноль
            &dwBytesRead,
            NULL
        ))
        {
            cerr << "ReadFile failed. Last error: " << GetLastError() << endl;
            break;
        }

        if (dwBytesRead == 0) continue;

        // Формируем строку из прочитанных данных (бинарно безопасно)
        string message(buffer, dwBytesRead);

        // Проверяем условие выхода в любом месте сообщения
        if (message.find("выход") != string::npos)
        {
            cout << "Exit command received. Stopping server." << endl;
            break;
        }

        // Проверка на команду FILE: формат "username: FILE <path> (<size> bytes)\n"
        // Ищем позицию первого вхождения ": FILE "
        size_t fileCmdPos = message.find(": FILE ");
        if (fileCmdPos != string::npos)
        {
            // Ищем первый перевод строки после заголовка
            size_t headerEnd = message.find('\n', fileCmdPos);
            if (headerEnd != string::npos)
            {
                // Заголовок от начала до headerEnd
                string header = message.substr(0, headerEnd);
                // Тело файла начинается после headerEnd + 1
                size_t dataStart = headerEnd + 1;

                // Извлекаем путь из заголовка: между ": FILE " и " ("
                size_t pathStart = fileCmdPos + 7; // длина ": FILE "
                size_t sizePos = header.find(" (", pathStart);
                if (sizePos != string::npos)
                {
                    string filepath = header.substr(pathStart, sizePos - pathStart);
                    // Получаем только имя файла
                    string filename = GetFileNameFromPath(filepath);
                    if (filename.empty()) filename = "unnamed";

                    // Формируем полный путь для сохранения
                    string savePath = "files\\" + filename;

                    // Сохраняем бинарные данные из message начиная с dataStart
                    FILE* pFile = fopen(savePath.c_str(), "wb");
                    if (pFile)
                    {
                        size_t written = fwrite(&message[dataStart], 1, message.size() - dataStart, pFile);
                        fclose(pFile);
                        cout << "File saved: " << savePath << " (" << written << " bytes)" << endl;
                    }
                    else
                    {
                        cerr << "Failed to create file: " << savePath << endl;
                    }
                    continue; // Переход к следующему сообщению
                }
            }
        }

        // Если не FILE, выводим как обычное сообщение
        cout << "Received: \"" << message << "\"" << endl;
    }

    delete[] buffer;
    CloseHandle(hMailslot);
    cout << "Server finished. Press any key to exit." << endl;
    cin.get();
    return 0;
}