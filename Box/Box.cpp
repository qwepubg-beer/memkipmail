#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Создаём почтовый ящик
    HANDLE hMailslot = CreateMailslot(
        "\\\\.\\mailslot\\demo_mailslot",
        0,                         // произвольный размер сообщения
        MAILSLOT_WAIT_FOREVER,     // бесконечное ожидание
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

    const DWORD BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    DWORD dwBytesRead;

    while (true)
    {
        // Синхронное чтение сообщения из ящика
        if (!ReadFile(
            hMailslot,
            buffer,
            BUFFER_SIZE - 1,   // оставляем место для завершающего нуля
            &dwBytesRead,
            NULL
        ))
        {
            cerr << "ReadFile failed. Last error: " << GetLastError() << endl;
            break;
        }

        // Формируем строку из прочитанных данных
        string message(buffer, dwBytesRead);
        cout << "Received: \"" << message << "\"  << endl;

        // Проверяем условие выхода
        if (message.find("выход") != string::npos)
        {
            cout << "Exit command received. Stopping server." << endl;
            break;
        }
    }

    CloseHandle(hMailslot);
    cout << "Server finished. Press any key to exit." << endl;
    cin.get();
    return 0;
}