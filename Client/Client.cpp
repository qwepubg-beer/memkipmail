#include <windows.h>
#include <iostream>
#include <string>
#include <stdio.h>   // fopen, fwrite, fclose
#include <locale.h>

using namespace std;

// Простая функция для получения имени файла из полного пути (без Shlwapi)
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

    // Создаём mailslot
    HANDLE hMailslot = CreateMailslot(
        "\\\\.\\mailslot\\demo_mailslot",
        0,                      // максимальный размер сообщения – 64 КБ
        MAILSLOT_WAIT_FOREVER,  // ждать бесконечно
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

    const DWORD BUFFER_SIZE = 65536;   // 64 КБ
    char* buffer = new char[BUFFER_SIZE];
    DWORD dwBytesRead;

    while (true)
    {
        if (!ReadFile(
            hMailslot,
            buffer,
            BUFFER_SIZE - 1,   // оставляем байт для возможного нуля (не обязательно)
            &dwBytesRead,
            NULL
        ))
        {
            cerr << "ReadFile failed. Last error: " << GetLastError() << endl;
            break;
        }

        if (dwBytesRead == 0)
            continue;

        // Бинарно-безопасная строка из полученных данных
        string message(buffer, dwBytesRead);

        // Проверяем команду "выход"
        if (message.find("выход") != string::npos)
        {
            cout << "Exit command received. Stopping server." << endl;
            break;
        }

        // Проверяем, является ли сообщение командой передачи файла
        // Формат: "username: FILE <путь> (<размер> bytes)\n<двоичные данные>"
        size_t fileCmdPos = message.find(": FILE ");
        if (fileCmdPos != string::npos)
        {
            // Ищем конец заголовка (символ перевода строки)
            size_t headerEnd = message.find('\n', fileCmdPos);
            if (headerEnd != string::npos)
            {
                string header = message.substr(0, headerEnd);
                size_t dataStart = headerEnd + 1; // начало данных файла

                // Извлекаем путь: между ": FILE " и " ("
                size_t pathStart = fileCmdPos + 7;  // длина ": FILE "
                size_t sizePos = header.find(" (", pathStart);
                if (sizePos != string::npos)
                {
                    string filepath = header.substr(pathStart, sizePos - pathStart);
                    string filename = GetFileNameFromPath(filepath);
                    if (filename.empty())
                        filename = "unnamed";

                    string savePath = "files\\" + filename;

                    // Сохраняем данные в файл
                    FILE* pFile = fopen(savePath.c_str(), "wb");
                    if (pFile)
                    {
                        size_t written = fwrite(&message[dataStart], 1,
                            message.size() - dataStart, pFile);
                        fclose(pFile);
                        cout << "File saved: " << savePath
                            << " (" << written << " bytes)" << endl;
                    }
                    else
                    {
                        cerr << "Failed to create file: " << savePath << endl;
                    }
                    continue; // обработали файл, переходим к следующему сообщению
                }
            }
        }

        // Обычное текстовое сообщение
        cout << "Received: \"" << message << "\"" << endl;
    }

    delete[] buffer;
    CloseHandle(hMailslot);
    cout << "Server finished. Press any key to exit." << endl;
    cin.get();
    return 0;
}