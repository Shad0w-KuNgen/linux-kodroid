#include "LauncherEngine.h"
#include <TlHelp32.h>
#define CURL_ICONV_CODESET_FOR_UTF8 "UTF-8"
#define PRINT_LOG [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }

Launcher::Launcher()
{
    mSocket = NULL;
    ready = false;
    m_dPercent = 0;
    m_bVersionGot = false;
    m_bPatchesGot = false;
    m_iVersion = 0;
    m_stateString = "Checking version and preparing to launch game.";

    GetCurrentDirectoryA(MAX_PATH, WorkingPath);
    std::string iniPath = std::string(WorkingPath) + "\\Server.ini";

    const size_t IPSize = 256;
    char sIP[IPSize] = { 0 };
    GetPrivateProfileStringA("Server", "IP0", "127.0.0.1", sIP, IPSize, iniPath.c_str());
    m_settingsIP = sIP;

    m_settingsVersion = (short)GetPrivateProfileIntA("Version", "Files", 1, iniPath.c_str());

    GetCurrentDirectoryA(MAX_PATH, m_strBasePath);
}

void Launcher::RequestVersion()
{
    if (mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return;

    int iOffset = 0;
    uint8_t byBuffs[1];
    CAPISocket::MP_AddByte(byBuffs, iOffset, 0x1);
    mSocket->Send(byBuffs, iOffset);
}

void Launcher::RequestPatch()
{
    if (mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return;

    int iOffset = 0;
    uint8_t byBuffs[3];
    CAPISocket::MP_AddByte(byBuffs, iOffset, 0x2);
    CAPISocket::MP_AddShort(byBuffs, iOffset, m_settingsVersion);
    mSocket->Send(byBuffs, iOffset);
}

void Launcher::RequestNotices()
{
    if (mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return;

    int iOffset = 0;
    uint8_t byBuffs[1];
    CAPISocket::MP_AddByte(byBuffs, iOffset, 0x3);
    mSocket->Send(byBuffs, iOffset);
}

bool Launcher::Start()
{
    // The actual TCP handshake below is a blocking call, so this function is
    // meant to run on its own worker thread (see StartConnectThread in
    // Launcher.cpp) rather than the UI thread. Everything that happens once
    // we're connected is asynchronous: CAPISocket::Connect() registers the
    // socket for WM_SOCKETMSG notifications on `window`, so all subsequent
    // receiving/packet handling happens on the UI thread's message loop
    // (WndProc's WM_SOCKETMSG case), not here.
    mSocket = new CAPISocket();
    SetState("Connecting to " + m_settingsIP + "...");
    int iErr = mSocket->Connect(window, m_settingsIP.c_str(), 15100);
    if (iErr)
    {
        SetState("Connection failed. Please retry connecting.");
        return false;
    }

    RequestVersion();
    return true;
}

void Launcher::Update()
{
    if (mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return;

    RequestNotices();

    m_bVersionGot = true;
    if (!m_bPatchesGot)
        Download();
}

double parseMB(double bytes)
{
    return bytes / 1024 / 1024;
}

int ProgCallback(void* ptr, double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded)
{
    if (Engine->mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return 0;

    if (dTotalToDownload > 0)
        Engine->SetPercent((uint8)round(dNowDownloaded * 100 / dTotalToDownload));
    Engine->SetState(std::format("Downloading {}: {:.2f}/{:.2f} MB.", Engine->m_currentFile.c_str(), parseMB(dNowDownloaded), parseMB(dTotalToDownload)));
    return 0;
}

int on_extract_entry(const char* filename, void* arg)
{
    Engine->SetState(std::format("Extracting: {}", filename));
    return 0;
}

bool Launcher::KnightOnlineCheck()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            std::string a = entry.szExeFile;
            if (a.find("KnightOnLine.exe") != std::string::npos)
            {
                CloseHandle(snapshot);
                return true;
            }
        }
    }

    CloseHandle(snapshot);
    return false;
}

bool Launcher::DownloadPatch(std::string server, std::string path, std::string file)
{
    if (mSocket->GetSocket() == (void*)INVALID_SOCKET)
        return false;

    if (m_settingsVersion < m_iVersion)
    {
        CFTPClient FTPClient(PRINT_LOG);
        FTPClient.InitSession(server, 21, "", "", CFTPClient::FTP_PROTOCOL::FTP, CFTPClient::ENABLE_LOG);
        FTPClient.SetProgressFnCallback(reinterpret_cast<void*>(0xFFFFFFFF), &ProgCallback);
        m_currentFile = file;
        FTPClient.DownloadFile(file, path + "/" + file);
        FTPClient.CleanupSession();
        std::string versionFromFile = m_currentFile.substr(0, m_currentFile.length() - 4);
        m_settingsVersion = (short)atoi(versionFromFile.c_str());
        Sleep(50);
        zip_extract(file.c_str(), WorkingPath, on_extract_entry, NULL);
        std::remove(file.c_str());
        WritePrivateProfileStringA("Version", "Files", std::to_string(m_settingsVersion).c_str(), (std::string(WorkingPath) + "\\Server.ini").c_str());
        return m_settingsVersion == m_iVersion;
    }
    return true;
}

void Launcher::Download()
{
    // Always ask the server what patches (if any) it has; we don't refuse to
    // proceed just because the local Server.ini version counter is ahead of
    // the server's protocol version - those are two different numbers.
    RequestPatch();
}

bool Launcher::LaunchGame()
{
    if (KnightOnlineCheck())
    {
        MessageBoxA(NULL, "KnightOnLine.exe is already running, please close it first.", "ISTIRAP", MB_ICONEXCLAMATION);
        return false;
    }

    std::string exePath = std::string(WorkingPath) + "\\KnightOnLine.exe";
    std::ifstream f(exePath);
    if (!f.good())
    {
        MessageBoxA(NULL, "KnightOnLine.exe not found.", "ISTIRAP", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }
    f.close();

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // The client (KnightOnLine.exe) refuses to run standalone: it checks its
    // command line for a launcher-supplied PID (shows "...(NullParam)" if the
    // command line is empty, "...(ProcID)" if it doesn't like the value), and
    // it also looks for its parent by the names "Launcher.exe"/"Launcher2.exe"
    // - see LauncherEngine.h's long-dead, never-wired-up `ipParam`/`ProcID`
    // fields for the original (incomplete) intent. We pass our own PID as the
    // command line; if the client still reports "(ProcID)", the check is more
    // specific than this and needs to be narrowed further from that error.
    std::string pidArg = std::to_string(GetCurrentProcessId());
    std::string cmdLine = "\"" + exePath + "\" " + pidArg;
    std::vector<char> cmdLineBuf(cmdLine.begin(), cmdLine.end());
    cmdLineBuf.push_back('\0');

    BOOL ok = CreateProcessA(
        exePath.c_str(),
        cmdLineBuf.data(),
        NULL, NULL, FALSE,
        0, NULL,
        WorkingPath,
        &si, &pi);

    if (!ok)
    {
        MessageBoxA(NULL, "Failed to start KnightOnLine.exe.", "ISTIRAP", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

bool Launcher::HandlePacket(Packet& pkt)
{
    int opCode = pkt.GetOpcode();

    switch (opCode)
    {
    case 0x1:
    {
        if (!m_bVersionGot)
        {
            pkt >> m_iVersion;
            Update();
        }
    }
    break;
    case 0x2:
    {
        std::string ftpURL, ftpPATH;
        uint16 fileCount = 0;
        pkt >> ftpURL >> ftpPATH >> fileCount;
        for (int i = 0; i < fileCount; i++)
        {
            std::string file;
            pkt >> file;
            DownloadPatch(ftpURL, ftpPATH, file);
        }

        if (fileCount > 0)
        {
            SetState("Files are being packed...");
            CHDRSystem* hdrPacker = new CHDRSystem;
            hdrPacker->Pack();
            delete hdrPacker;
        }

        m_bPatchesGot = true;
        SetPercent(100);
        SetState("Ready to play.");
        ready = true;
    }
    break;
    case 0x3:
    {
        uint16 noticeCount;
        pkt >> noticeCount;
        std::string notice = "";
        for (uint16 i = 0; i < noticeCount; i++)
        {
            pkt >> notice;
            m_lNotices.push_back(notice);
        }
        std::reverse(m_lNotices.begin(), m_lNotices.end());
    }
    break;
    default:
        break;
    }

    return true;
}

Launcher::~Launcher()
{
    if (mSocket)
        mSocket->Release();
}
