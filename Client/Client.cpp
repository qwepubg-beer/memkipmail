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
        getline(cin, message);

        if (message.find("выход") != string::npos)
            break;

        DWORD dwBytesWritten;
        if (!WriteFile(
            hMailslot,
            message.c_str(),
            message.size(),
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
    }
    CloseHandle(hMailslot);
    cout << "Client finished. Press any key to exit." << endl;
    cin.get();
    return 0;
}