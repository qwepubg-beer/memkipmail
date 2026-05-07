#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    char mailslotName[] = "\\\\*\\mailslot\\demo_mailslot";
    HANDLE hMailslot;

    // Открываем существующий почтовый ящик для записи
    hMailslot = CreateFile(
        mailslotName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hMailslot == INVALID_HANDLE_VALUE)
    {
        cerr << "CreateFile failed. Last error: " << GetLastError() << endl;
        cout << "Press any key to exit.";
        cin.get();
        return 1;
    }

    cout << "Connected to mailslot. Type messages (enter 'выход' to quit)." << endl;

    string message;
    while (true)
    {
        cout << "Message: ";
        getline(cin, message); // используем getline для возможности пробелов

        if (message.find("выход") != string::npos)
            break;

        DWORD dwBytesWritten;
        // Передаём данные как массив символов (без завершающего нуля можно по желанию)
        if (!WriteFile(
            hMailslot,
            message.c_str(),
            message.size(),               // реальная длина сообщения
            &dwBytesWritten,
            NULL
        ))
        {
            cerr << "WriteFile failed. Last error: " << GetLastError() << endl;
            CloseHandle(hMailslot);
            cout << "Press any key to exit.";
            cin.get();
            return 1;
        }

        cout << "Message sent (" << dwBytesWritten << " bytes)." << endl;

        // Очищаем поток после предыдущего ввода, если были пробелы и т.п.
        // (при getline это не нужно, но оставим для совместимости)
    }

    CloseHandle(hMailslot);
    cout << "Client finished. Press any key to exit." << endl;
    cin.get();
    return 0;
}