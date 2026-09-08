#include "IOCompletionPort.h"
//#include"Packet.h"
#include "Math.h"
#include <iostream>
#include <mswsock.h> // AcceptEx
#include <DirectXMath.h>
#include<random>
#include<cmath>
#pragma comment(lib, "Mswsock.lib")

using namespace DirectX;

// 맵 데이터를 읽기 위한 스크립트 객체
ScriptUtil terrainScript;

// 현재 로드된 터레인 정점 벡터
std::vector<XMFLOAT3> terrainData;

// 몬스터 생성 데이터를 읽기 위한 스크립트 객체
ScriptUtil monsterDataScript;

// 현재 로드된 몬스터 생성 타입 및 위치
std::vector<MonsterData> monsterData;

// 몬스터 생성 데이터를 읽기 위한 스크립트 객체2
ScriptUtil monsterDataScript2;

// 현재 로드된 몬스터 생성 타입 및 위치2
std::vector<MonsterData> monsterData2;

// 몬스터 생성 데이터를 읽기 위한 스크립트 객체3
ScriptUtil monsterDataScript3;

// 현재 로드된 몬스터 생성 타입 및 위치3
std::vector<MonsterData> monsterData3;


std::mutex roomMapMutex;
//std::vector<MonsterData> myMonsters;//임시

//std::vector<MonsterData> defenseMonsters(DEFENSE_MONSTER);//임시

//std::mutex defenseMonsterMutex;//임시

//bool defenseState = true;//임시

//int clearCount = 0;//임시


//int roomHp= CENTER_HP;//임시


// NPC 스레드 프레임 시간
float deltaTime{};

bool randomTreadFlag1 = true;//임시
bool randomTreadFlag2 = true;
bool randomTreadFlag3 = true;
extern LPFN_ACCEPTEX lpfnAcceptEx = nullptr; // 전역으로 AcceptEx 포인터
IOCompletionPort::IOCompletionPort() {}

IOCompletionPort::~IOCompletionPort() {
    WSACleanup();
}

bool IOCompletionPort::InitSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        return false;
    }

    // AcceptEx 가져오기
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytes;
    WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &bytes, NULL, NULL);

    std::cout << "소켓 초기화 성공\n";
    return true;
}

bool IOCompletionPort::BindandListen() {
    SOCKADDR_IN serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        return false;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        return false;
    }

    std::cout << "서버 등록 성공..\n";
    return true;
}
void IOCompletionPort::PostAccept() {
    stOverlappedEx* overlappedEx = new stOverlappedEx();
    overlappedEx->operation = IOOperation::ACCEPT;
    ZeroMemory(&overlappedEx->overlapped, sizeof(overlappedEx->overlapped));
    overlappedEx->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    DWORD bytesReceived = 0;
    int addrLen = sizeof(SOCKADDR_IN) + 16;

    BOOL ret = lpfnAcceptEx(
        listenSocket,
        overlappedEx->acceptSocket,
        overlappedEx->buffer,
        0,
        addrLen,
        addrLen,
        &bytesReceived,
        &overlappedEx->overlapped
    );

    if (ret == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
        std::cerr << "AcceptEx 실패: " << WSAGetLastError() << "\n";
        delete overlappedEx;
    }
}

bool IOCompletionPort::StartServer() {
    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
    if (iocpHandle == NULL) {
        std::cerr << "[에러] CreateIoCompletionPort() 실패: " << GetLastError() << "\n";
        return false;
    }


    // 맵 터레인 데이터 로드
    terrainScript.Release();
    terrainData.clear();

    terrainScript.Load("mapData//terrain_map1.xml");
    auto LoadBehavior = [&](CategoryPtr Category)
        {
            XMFLOAT3 loadedVertex{};
            loadedVertex.x = terrainScript.LoadDigitData(Category, "x");
            loadedVertex.y = terrainScript.LoadDigitData(Category, "y");
            loadedVertex.z = terrainScript.LoadDigitData(Category, "z");
            terrainData.emplace_back(loadedVertex);
        };
    terrainScript.LoadAllData(LoadBehavior);


    // 몬스터 타입 및 생성 위치 로드-1
    monsterDataScript.Release();
    monsterData.clear();

    monsterDataScript.Load("mapData//monster_map1.xml");
    unsigned int monsterIdCounter = 0;
    auto LoadMonsterData = [&](CategoryPtr Category)
        {
            MonsterData loadedData{};
            loadedData.monsterType = (int)monsterDataScript.LoadDigitData(Category, "type");
            loadedData.createPointX = monsterDataScript.LoadDigitData(Category, "x");
            loadedData.createPointZ = monsterDataScript.LoadDigitData(Category, "z");
            loadedData.id = monsterIdCounter++; // 고유 ID 부여
            if (loadedData.monsterType == PLANT_TYPE) {
                loadedData.hp = PLANT_HP;
            }
            else if (loadedData.monsterType == SCORPION_TYPE) {
                loadedData.hp = SCORPION_HP;
            }

            monsterData.emplace_back(loadedData);
        };
    monsterDataScript.LoadAllData(LoadMonsterData);
    std::cout << "monster_map1.xml로드 완료!\n";
    // 몬스터 타입 및 생성 위치 로드-2
    monsterDataScript2.Release();
    monsterData2.clear();

    monsterDataScript2.Load("mapData//monster_map2.xml");/////////--------------------------받아야함
    monsterIdCounter = 0;
    auto LoadMonsterData2 = [&](CategoryPtr Category)
        {
            MonsterData loadedData2{};
            loadedData2.monsterType = (int)monsterDataScript2.LoadDigitData(Category, "type");
            loadedData2.createPointX = monsterDataScript2.LoadDigitData(Category, "x");
            loadedData2.createPointZ = monsterDataScript2.LoadDigitData(Category, "z");
            loadedData2.id = monsterIdCounter++; // 고유 ID 부여
            if (loadedData2.monsterType == TROLL_TYPE) {
                loadedData2.hp = TROLL_HP;
            }
            else if (loadedData2.monsterType == TREANT_TYPE) {
                loadedData2.hp = TREANT_HP;
            }
            monsterData2.emplace_back(loadedData2);
        };
    monsterDataScript2.LoadAllData(LoadMonsterData2);
    std::cout << "monster_map2.xml로드 완료!\n";
    // 몬스터 타입 및 생성 위치 로드-3
    monsterDataScript3.Release();
    monsterData3.clear();

    monsterDataScript3.Load("mapData//monster_map3.xml");/////////--------------------------받아야함
    monsterIdCounter = 0;
    auto LoadMonsterData3 = [&](CategoryPtr Category)
        {
            MonsterData loadedData3{};
            loadedData3.monsterType = (int)monsterDataScript3.LoadDigitData(Category, "type");
            loadedData3.createPointX = monsterDataScript3.LoadDigitData(Category, "x");
            loadedData3.createPointZ = monsterDataScript3.LoadDigitData(Category, "z");
            loadedData3.id = monsterIdCounter++; // 고유 ID 부여
            if (loadedData3.monsterType == IMP_TYPE) {
                loadedData3.hp = IMP_HP;
            }
            else if (loadedData3.monsterType == GAZER_TYPE) {
                loadedData3.hp = GAZER_HP;
            }
            monsterData3.emplace_back(loadedData3);
        };
    monsterDataScript3.LoadAllData(LoadMonsterData3);
    std::cout << "monster_map3.xml로드 완료!\n";
    //
    CreateIoCompletionPort((HANDLE)listenSocket, iocpHandle, 9999, 0);
    PostAccept();
    workerThread = std::thread([this]() { WorkThread(); });
   //
    randomPositionThread = std::thread([this]() { RandomPositionThread(); });
    randomPositionThread2 = std::thread([this]() { RandomPositionThread2(); });
    randomPositionThread3 = std::thread([this]() { RandomPositionThread3(); });
    timerService = std::make_unique<TimerService>(*this);
    timerService->start();
    
    std::cout << "서버가 시작되었습니다.\n";
    return true;
}
std::random_device RD;
XMFLOAT2 GenPointInDonut(float DiameterMin, float DiameterMax, const XMFLOAT2& Center) {
    static std::default_random_engine dre(RD());
    static std::uniform_real_distribution<float> DistAngle(0.0f, 2.0f * XM_PI);
    static std::uniform_real_distribution<float> DistRadius(0.0f, 1.0f);

    float Angle = DistAngle(dre);
    float RadiusMin = DiameterMin * 0.5;
    float RadiusMax = DiameterMax * 0.5;
    float R = std::sqrt((RadiusMax * RadiusMax - RadiusMin * RadiusMin) * DistRadius(dre) + RadiusMin * RadiusMin);

    float X = R * std::cosf(Angle) + Center.x;
    float Y = R * std::sinf(Angle) + Center.y;
    return XMFLOAT2(X, Y);
    //GenPointInDonut(30.0,60.0,XMFLOAT2(-120.0,-120.0));
}


void IOCompletionPort::RandomPositionThread() {
    using namespace std::chrono;

    const seconds sendInterval(CREAT_MONSTER_TIME);  // 몬스터 1마리당 간격

    while (isRunning) {
        auto now = steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(roomMapMutex);

            for (auto& [roomID, room] : rooms) {
                if (!room.defenseState || room.stageState != 1 || room.monsterRandomSent)
                    continue;

                // 3초 지연 시작 설정 (딱 한 번만)
                if (room.defenseStartTime == steady_clock::time_point::min()) {
                    room.defenseStartTime = now + seconds(PASS_STAGE_TIME);
                    room.lastMonsterSendTime = room.defenseStartTime;
                    std::cout << "[1s-Room " << roomID << "] 몬스터 전송 n초 후 시작 예정!!!!!!!!!!!!!!!!!!!!!!\n";
                }

                if (now < room.defenseStartTime)
                    continue;

                if (now - room.lastMonsterSendTime < sendInterval)
                    continue;

                int monsterId = room.nextMonsterToSend;

                if (monsterId >= DEFENSE_MONSTER1) {
                    room.monsterRandomSent = true;
                    std::cout << "[1s-Room " << roomID << "] 모든 몬스터 전송 완료\n";
                    continue;
                }

                float x = room.defenseMonsters[monsterId].createPointX;
                float z = room.defenseMonsters[monsterId].createPointZ;

                for (auto* client : room.clients) {
                    if (!client || client->alreadyRemoved || client->socketClient == INVALID_SOCKET)
                        continue;

                    DefenseRandomPacket* packet = new DefenseRandomPacket{};
                    packet->type = PacketType::RANDOM_POSITION;
                    packet->monsterID = monsterId;
                    packet->x = x;
                    packet->z = z;

                    stOverlappedEx* sendOver = new stOverlappedEx{};
                    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
                    sendOver->operation = IOOperation::SEND;
                    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
                    sendOver->wsaBuf.len = sizeof(DefenseRandomPacket);
                    sendOver->cleanup = [packet, sendOver]() {
                        delete packet;
                        delete sendOver;
                        };

                    std::cout << "[1s-Room " << roomID << "] 몬스터 " << monsterId << " 위치 전송\n";

                    int ret = WSASend(client->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
                        &sendOver->overlapped, NULL);
                    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                        std::cerr << "[에러] 랜덤 위치 전송 실패: " << WSAGetLastError() << std::endl;
                        PostRemove(client);
                    }
                }

                // 다음 몬스터로 넘어감
                room.nextMonsterToSend++;
                room.lastMonsterSendTime = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void IOCompletionPort::RandomPositionThread2() {
    using namespace std::chrono;
    std::cout << "RandomPositionThread2시작\n";
    const seconds sendInterval(CREAT_MONSTER_TIME);  // 몬스터 1마리당 간격

    while (isRunning) {
        auto now = steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(roomMapMutex);

            for (auto& [roomID, room] : rooms) {
                if (!room.defenseState || room.stageState != 2 || room.monsterRandomSent)
                    continue;

                // 3초 지연 시작 설정 (딱 한 번만)
                if (room.defenseStartTime == steady_clock::time_point::min()) {
                    room.defenseStartTime = now + seconds(PASS_STAGE_TIME);
                    room.lastMonsterSendTime = room.defenseStartTime;
                    std::cout << "[2s-Room " << roomID << "] 몬스터 전송 3초 후 시작 예정\n";
                }

                if (now < room.defenseStartTime)
                    continue;

                if (now - room.lastMonsterSendTime < sendInterval)
                    continue;

                int monsterId = room.nextMonsterToSend;

                if (monsterId >= DEFENSE_MONSTER1) {
                    room.monsterRandomSent = true;
                    std::cout << "[2s-Room " << roomID << "] 모든 몬스터 전송 완료\n";
                    continue;
                }

                float x = room.defenseMonsters2[monsterId].createPointX;
                float z = room.defenseMonsters2[monsterId].createPointZ;

                for (auto* client : room.clients) {
                    if (!client || client->alreadyRemoved || client->socketClient == INVALID_SOCKET)
                        continue;

                    DefenseRandomPacket* packet = new DefenseRandomPacket{};
                    packet->type = PacketType::RANDOM_POSITION;
                    packet->monsterID = monsterId;
                    packet->x = x;
                    packet->z = z;

                    stOverlappedEx* sendOver = new stOverlappedEx{};
                    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
                    sendOver->operation = IOOperation::SEND;
                    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
                    sendOver->wsaBuf.len = sizeof(DefenseRandomPacket);
                    sendOver->cleanup = [packet, sendOver]() {
                        delete packet;
                        delete sendOver;
                        };

                    std::cout << "[2s-Room " << roomID << "] 몬스터 " << monsterId << " 위치 전송\n";

                    int ret = WSASend(client->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
                        &sendOver->overlapped, NULL);
                    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                        std::cerr << "[에러] 랜덤 위치 전송 실패: " << WSAGetLastError() << std::endl;
                        PostRemove(client);
                    }
                }

                // 다음 몬스터로 넘어감
                room.nextMonsterToSend++;
                room.lastMonsterSendTime = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void IOCompletionPort::RandomPositionThread3() {
    using namespace std::chrono;
    std::cout << "RandomPositionThread3시작\n";
    const seconds sendInterval(CREAT_MONSTER_TIME);  // 몬스터 1마리당 간격

    while (isRunning) {
        auto now = steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(roomMapMutex);

            for (auto& [roomID, room] : rooms) {
                if (!room.defenseState || room.stageState != 3 || room.monsterRandomSent)
                    continue;

                // 3초 지연 시작 설정 (딱 한 번만)
                if (room.defenseStartTime == steady_clock::time_point::min()) {
                    room.defenseStartTime = now + seconds(PASS_STAGE_TIME);
                    room.lastMonsterSendTime = room.defenseStartTime;
                    std::cout << "[3s-Room " << roomID << "] 몬스터 전송 3초 후 시작 예정\n";
                }

                if (now < room.defenseStartTime)
                    continue;

                if (now - room.lastMonsterSendTime < sendInterval)
                    continue;

                int monsterId = room.nextMonsterToSend;

                if (monsterId >= DEFENSE_MONSTER1) {
                    room.monsterRandomSent = true;
                    std::cout << "[3s-Room " << roomID << "] 모든 몬스터 전송 완료\n";
                    continue;
                }

                float x = room.defenseMonsters3[monsterId].createPointX;
                float z = room.defenseMonsters3[monsterId].createPointZ;

                for (auto* client : room.clients) {
                    if (!client || client->alreadyRemoved || client->socketClient == INVALID_SOCKET)
                        continue;

                    DefenseRandomPacket* packet = new DefenseRandomPacket{};
                    packet->type = PacketType::RANDOM_POSITION;
                    packet->monsterID = monsterId;
                    packet->x = x;
                    packet->z = z;

                    stOverlappedEx* sendOver = new stOverlappedEx{};
                    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
                    sendOver->operation = IOOperation::SEND;
                    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
                    sendOver->wsaBuf.len = sizeof(DefenseRandomPacket);
                    sendOver->cleanup = [packet, sendOver]() {
                        delete packet;
                        delete sendOver;
                        };

                    std::cout << "[3s-Room " << roomID << "] 몬스터 " << monsterId << " 위치 전송\n";

                    int ret = WSASend(client->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
                        &sendOver->overlapped, NULL);
                    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                        std::cerr << "[에러] 랜덤 위치 전송 실패: " << WSAGetLastError() << std::endl;
                        PostRemove(client);
                    }
                }

                // 다음 몬스터로 넘어감
                room.nextMonsterToSend++;
                room.lastMonsterSendTime = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void IOCompletionPort::SendData_MonsterMoveToAllClients(const MonsterData& m) {
    
    for (auto* client : clients) {
        if (!client || client->alreadyRemoved || client->socketClient == INVALID_SOCKET)
            continue;
        MovePacket_StoC* packet = new MovePacket_StoC{};
        packet->type = PacketType::MONSTER_MOVE;  // 필요시 MONSTER_MOVE로 별도 정의 가능
        packet->id = m.id;
        packet->x = m.createPointX;
        packet->y = 0.0f;  // 고정값 혹은 높이 정보가 있다면 그 값으로
        packet->z = m.createPointZ;
        stOverlappedEx* sendOver = new stOverlappedEx{};
        ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
        sendOver->operation = IOOperation::SEND;
        sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
        sendOver->wsaBuf.len = sizeof(MovePacket_StoC);
        sendOver->cleanup = [packet, sendOver]() {
           // std::cout << "[디버그] cleanup called for monster move packet\n";
            delete packet;
            delete sendOver;
         };

        int ret = WSASend(client->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
            &sendOver->overlapped, NULL);

        //if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        //    std::cerr << "[에러] WSASend 실패(MonsterMove): " << WSAGetLastError() << std::endl;
        //    closesocket(client->socketClient);
        //    RemoveClient(client);
        //    delete packet;
        //    delete sendOver;
        //}
        if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "[에러] WSASend 실패(MonsterMove): " << WSAGetLastError() << std::endl;

            if (!client->alreadyRemoved) {
                closesocket(client->socketClient);
                PostRemove(client);
            }

            // cleanup 람다 등록했으므로 delete 중복 금지
            // delete packet;
            // delete sendOver;
        }
    }
}



void IOCompletionPort::CreateRoom(const std::vector<stClientInfo*>& members) {
    //std::lock_guard<std::mutex> lock(roomMutex);

    Room newRoom;
    newRoom.roomID = nextRoomID;//=0
    newRoom.clients = members;
    //newRoom.defenseStartTime = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    newRoom.lastMonsterSendTime = newRoom.defenseStartTime;
    std::cout << "---newRoom.roomID:" << newRoom.roomID << std::endl;
    nextRoomID++;
    for (const auto& m : monsterData) {
        MonsterData copy = m;
        newRoom.myMonsters.push_back(copy);
    }
    for (const auto& m : monsterData2) {
        MonsterData copy = m;
        newRoom.myMonsters2.push_back(copy);
    }
    for (const auto& m : monsterData3) {
        MonsterData copy = m;
        newRoom.myMonsters3.push_back(copy);
    }
    //->이거에 대한 접근 예시
    //auto& room = rooms[2];
    //if (!room.monsters.empty()) {
    //    std::cout << "HP: " << room.monsters[0].hp << std::endl;
    //}

    for (int i = 0; i < DEFENSE_MONSTER1; ++i) {
        MonsterData def;
        def.id = i;
        def.state = 0;
        def.monsterType = PLANT_TYPE;
        def.hp = PLANT_HP;
        XMFLOAT2 rendomPosition = GenPointInDonut(MAP1_RANDOM_MIN_RADIANS, MAP1_RANDOM_MAX_RADIANS, XMFLOAT2(-120.0, -120.0));//----------------------
        def.createPointX = rendomPosition.x;
        def.createPointZ = rendomPosition.y;
        newRoom.defenseMonsters[i] = def;
    }
    for (int i = 0; i < DEFENSE_MONSTER2; ++i) {
        MonsterData def;
        def.id = i;
        def.state = 0;
        def.monsterType = TREANT_TYPE;
        def.hp = TREANT_HP;
        XMFLOAT2 rendomPosition = GenPointInDonut(MAP2_RANDOM_MIN_RADIANS, MAP2_RANDOM_MAX_RADIANS, XMFLOAT2(-120.0, -120.0));
        def.createPointX = rendomPosition.x;
        def.createPointZ = rendomPosition.y;
        newRoom.defenseMonsters2[i] = def;
    }
    for (int i = 0; i < DEFENSE_MONSTER3; ++i) {
        MonsterData def;
        def.id = i;
        def.state = 0;
        def.monsterType = IMP_TYPE;
        def.hp = IMP_HP;
        XMFLOAT2 rendomPosition = GenPointInDonut(MAP3_RANDOM_MIN_RADIANS, MAP3_RANDOM_MAX_RADIANS, XMFLOAT2(-120.0, -120.0));
        def.createPointX = rendomPosition.x;
        def.createPointZ = rendomPosition.y;
        newRoom.defenseMonsters3[i] = def;
    }

    for (auto* client : members) {
        client->roomID = newRoom.roomID;
        // 방 입장 메시지 전송
        //SendRoomEnterMessage(client, newRoom.roomID);
    }

    // centerHp, clearCount, defenseState 기본 설정
    newRoom.centerHp = CENTER_HP;
    newRoom.clearCount = 0;
    //newRoom.defenseState = false;
    //rooms[newRoom.roomID] = std::move(newRoom);
    {
        std::lock_guard<std::mutex> lock(roomMapMutex);
        rooms[newRoom.roomID] = std::move(newRoom);
    }
    std::cout << "create room!: " << newRoom.roomID << std::endl;

    //if (randomTreadFlag ) {
    //    randomPositionThread = std::thread([this]() { RandomPositionThread(); });
    //    randomTreadFlag = false;
    //}
}

void IOCompletionPort::SendData_EnterRoom(stClientInfo* receiver) {
    EnterRoomPacket* packet = new EnterRoomPacket{};
    packet->type = PacketType::ENTER;
    packet->roomID = receiver->roomID;
    packet->myID = receiver->id;
    std::cout << "SendData_EnterRoom ID:" << receiver->id << std::endl;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
    sendOver->wsaBuf.len = sizeof(EnterRoomPacket);
    sendOver->cleanup = [packet, sendOver]() {delete packet; delete sendOver; };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cout << "ERROR-SendData_EnterRoom to ID: " << receiver->id << " / room: " << receiver->roomID << std::endl;
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete packet;
        delete sendOver;
    }
}

void IOCompletionPort::RegisterRecv(stClientInfo* client) {
    if (!client || client->socketClient == INVALID_SOCKET) {
        std::cerr << "[에러] 유효하지 않은 클라이언트 소켓\n";
        return;
    }

    DWORD flags = 0;
    DWORD bytesReceived = 0;

    stOverlappedEx* recvOver = new stOverlappedEx{};
    ZeroMemory(recvOver, sizeof(stOverlappedEx));

    recvOver->operation = IOOperation::RECV;
    recvOver->wsaBuf.len = MAX_SOCKBUF;
    recvOver->wsaBuf.buf = recvOver->buffer;

    // ✅ cleanup 콜백 등록
    recvOver->cleanup = [recvOver]() {
        delete recvOver;
        };

    int result = WSARecv(
        client->socketClient,
        &recvOver->wsaBuf,
        1,
        &bytesReceived,
        &flags,
        &recvOver->overlapped,
        NULL
    );

    //std::cout << "[DEBUG] RegisterRecv 요청\n";

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "[에러] WSARecv 실패: " << WSAGetLastError() << std::endl;
        closesocket(client->socketClient);
        PostRemove(client);
        recvOver->cleanup();  // 실패 시 즉시 해제
    }
}

bool IOCompletionPort::AddClient(stClientInfo* c)
{
    for (auto& slot : clients)
        if (slot == nullptr) { slot = c; ++clientCount; return true; }
    std::cerr << "[경고] MAX_CLIENTS 초과\n";
    return false;
}

void IOCompletionPort::RemoveClient(stClientInfo* c)
{
    if (!c) return;

    // 중복 제거 요청 차단은 PostRemove에서 처리 (진입 경로가 ProcessDelayedRemoves 단독)

    // ②소켓 닫기
    if (c->socketClient != INVALID_SOCKET) {
        shutdown(c->socketClient, SD_BOTH);
        closesocket(c->socketClient);
        c->socketClient = INVALID_SOCKET;
    }

    //  전체 클라이언트 목록에서 제거
    for (auto& slot : clients)
        if (slot == c) { slot = nullptr; --clientCount; break; }

    std::cout << "clientCount: " << clientCount << std::endl;

    //  웨이팅룸에서 제거 
    {
        std::lock_guard<std::mutex> lock(waitMutex);
        waitingClients.erase(
            std::remove(waitingClients.begin(), waitingClients.end(), c),
            waitingClients.end());
    }

    //  방(Room)의 clients 목록에서도 제거 
    {
        std::lock_guard<std::mutex> lock(roomMapMutex);
        auto it = rooms.find(c->roomID);
        if (it != rooms.end()) {
            Room& room = it->second;
            room.clients.erase(
                std::remove(room.clients.begin(), room.clients.end(), c),
                room.clients.end());
        }
    }

    // ⑥ 최종 메모리 해제
    delete c;
}

void IOCompletionPort::PostRemove(stClientInfo* client) {
    if (!client || client->alreadyRemoved.exchange(true))
        return;

    DelayedRemoveEntry entry;
    entry.client = client;
    entry.timeScheduled = std::chrono::steady_clock::now() + std::chrono::milliseconds(100); // 100ms 후 제거

    std::lock_guard<std::mutex> lock(removeQueueMutex);
    removeQueue.push_back(entry);
}
void IOCompletionPort::ProcessDelayedRemoves() {
    auto now = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(removeQueueMutex);
    auto it = removeQueue.begin();

    while (it != removeQueue.end()) {
        if (now >= it->timeScheduled) {
            std::cout << "[딜레이 제거] 클라이언트 ID: " << it->client->id << " 제거 실행\n";
            RemoveClient(it->client);
            it = removeQueue.erase(it);
        }
        else {
            ++it;
        }
    }
}
void IOCompletionPort::SendData_Move(stClientInfo* sender, stClientInfo* receiver) {
    MovePacket_StoC* packet = new MovePacket_StoC{};
    packet->type = PacketType::MOVE;
    packet->id = sender->id;
    packet->x = sender->x;
    packet->y = sender->y;
    packet->z = sender->z;

    stOverlappedEx* sendOver = new stOverlappedEx();
    ZeroMemory(&sendOver->overlapped, sizeof(WSAOVERLAPPED));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
    sendOver->wsaBuf.len = sizeof(MovePacket_StoC);
    sendOver->cleanup = [packet, sendOver]() {
        delete packet;
        delete sendOver;
        };
    DWORD sizeSent = 0;
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, &sizeSent, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "[에러] WSASend 실패(MOVE): " << WSAGetLastError() << std::endl;
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete packet;
        delete sendOver;
    }
}


void IOCompletionPort::SendData_ViewAngle(stClientInfo* sender, stClientInfo* receiver) {
    ViewingAnglePacket_StoC* packet = new ViewingAnglePacket_StoC{};
    packet->type = PacketType::VIEW_ANGLE;
    packet->id = sender->id;
    packet->x = sender->angle_x;
    packet->y = sender->angle_y;
    packet->z = sender->angle_z;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
    sendOver->wsaBuf.len = sizeof(ViewingAnglePacket_StoC);

    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    sendOver->cleanup = [packet, sendOver]() {
        delete packet;
        delete sendOver;
        };
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete packet;
        delete sendOver;
    }
}

void IOCompletionPort::SendData_Animaion(stClientInfo* sender, stClientInfo* receiver) {
    AnimationPacket_StoC* packet = new AnimationPacket_StoC{};
    packet->type = PacketType::ANIMATION;
    packet->id = sender->id;
    packet->animationType = sender->animationType;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
    sendOver->wsaBuf.len = sizeof(AnimationPacket_StoC);
    sendOver->cleanup = [packet, sendOver]() {
        delete packet;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete packet;
        delete sendOver;
    }
}



void IOCompletionPort::SendData(stClientInfo* sender, stClientInfo* receiver, const char* message, int length) {
    ChatPacket_StoC* packet = new ChatPacket_StoC{};
    packet->type = PacketType::CHAT;
    packet->id = sender->id;
    memcpy(packet->message, message, length - sizeof(PacketType));

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(packet);
    sendOver->wsaBuf.len = length + sizeof(unsigned int);
    sendOver->cleanup = [packet, sendOver]() {
        delete packet;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete packet;
        delete sendOver;
    }
}

void IOCompletionPort::SendExistingClientsToNewClient(stClientInfo* receiver) {
    if (receiver == nullptr)
        return;

    std::cout << "SendExistingClientsToNewClient:\n";

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        stClientInfo* client = clients[i];

        if (client == nullptr) continue;
        if (client == receiver) continue;
        if (client->roomID != receiver->roomID) continue;

        ExistingClientsDataPacket* pkt = new ExistingClientsDataPacket{};
        pkt->type = PacketType::EXISTING_CLIENTS;
        pkt->id = client->id;
        pkt->size = client->name_size;
        memcpy(pkt->name,client->name,client->name_size);
        pkt->name[pkt->size] = '\0';

        stOverlappedEx* sendOver = new stOverlappedEx{};
        ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
        sendOver->operation = IOOperation::SEND;
        sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
        sendOver->wsaBuf.len = sizeof(ExistingClientsDataPacket);
        sendOver->cleanup = [pkt, sendOver]() {
            delete pkt;
            delete sendOver;
            };

        std::cout << "→ 기존 클라 ID 전송: " << pkt->id << "\n";

        int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
            &sendOver->overlapped, NULL);

        if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "[SendExistingClientsToNewClient] WSASend 실패: ID " << client->id << "\n";
            closesocket(receiver->socketClient);
            PostRemove(receiver);
            delete pkt;
            delete sendOver;
        }
    }
}
void IOCompletionPort::NotifyOthersAboutNewClient(stClientInfo* newClient) {
    for (int i = 0; i < clientCount;i++) {
        if (!clients[i] || clients[i] == newClient || clients[i]->roomID != newClient->roomID) continue;

        NewClientPacket* pkt = new NewClientPacket{};
        pkt->type = PacketType::NEW_CLIENT;
        pkt->id = newClient->id;
        pkt->size = newClient->name_size;
        memcpy(pkt->name, newClient->name, newClient->name_size);
        pkt->name[pkt->size] = '\0';

        std::cout << "NotifyOthersAboutNewClient-id:" << pkt->id << std::endl;


        stOverlappedEx* sendOver = new stOverlappedEx{};
        ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
        sendOver->operation = IOOperation::SEND;
        sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
        sendOver->wsaBuf.len = sizeof(NewClientPacket);
        sendOver->cleanup = [pkt, sendOver]() {
            delete pkt;
            delete sendOver;
        };
        int ret = WSASend(clients[i]->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
            &sendOver->overlapped,
            NULL);

        if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            closesocket(clients[i]->socketClient);
            PostRemove(clients[i]);
            delete pkt;
            delete sendOver;
        }
    }
}
void IOCompletionPort::SendData_MonsterState(stClientInfo* receiver,unsigned int monsterType, unsigned int monsterState, unsigned int id) {
        MonsterStatePacket_StoC* pkt = new MonsterStatePacket_StoC{};
        pkt->Ptype = PacketType::MONSTER_STATE;
        //pkt->Mtype = monsterType;
        pkt->state = monsterState;
        pkt->id = id;

        stOverlappedEx* sendOver = new stOverlappedEx{};
        ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
        sendOver->operation = IOOperation::SEND;
        sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
        sendOver->wsaBuf.len = sizeof(MonsterStatePacket_StoC);
        sendOver->cleanup = [pkt, sendOver]() {
            delete pkt;
            delete sendOver;
            };
        int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
            &sendOver->overlapped,
            NULL);

        if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            closesocket(receiver->socketClient);
            PostRemove(receiver);
            delete pkt;
            delete sendOver;
        }
}
void IOCompletionPort::SendData_MonsterMove(stClientInfo* receiver,float x, float y, float z, float angle, unsigned int monsterId,unsigned int playerId) {
    MonsterMovePacket* pkt = new MonsterMovePacket{};
    pkt->type = PacketType::MONSTER_MOVE;
    pkt->x = x;
    pkt->y = y;
    pkt->z = z;
    pkt->angle_y= angle;
    pkt->playerId = playerId;
    pkt->monsterId = monsterId;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(MonsterMovePacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_MtoPDamagePacket(stClientInfo* receiver, unsigned int playerID, unsigned int monsterID, int attackHp) {
    MtoPDamagePacket* pkt = new MtoPDamagePacket{};
    pkt->type = PacketType::MTOP_DAMAGE;
    pkt->playerID = playerID;
    pkt->monsterID = monsterID;
    pkt->attackHp = attackHp;
   

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(MtoPDamagePacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}

void IOCompletionPort::SendData_PtoMDamagePacket(stClientInfo* receiver,unsigned int monsterID,int attackHp) {
    PtoMDamagePacket* pkt = new PtoMDamagePacket{};
    pkt->type = PacketType::PTOM_DAMAGE;
    pkt->monsterID = monsterID;
    pkt->attackHp = attackHp;


    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(PtoMDamagePacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_EngineerInstallPacket(stClientInfo* receiver, int type,unsigned int ID,float rotY,float posX,float posY,float posZ) {
    EngineerInstallPacket* pkt = new EngineerInstallPacket{};
    pkt->Ptype = PacketType::ENGINEER_INSTALL;
    pkt->Etype = type;
    pkt->ID = ID;
    pkt->posX = posX;
    pkt->posY = posY;
    pkt->posZ = posZ;
    pkt->rotY = rotY;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(EngineerInstallPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_EngineerObjectPacket(stClientInfo* receiver, unsigned int ID,int hp) {
    EngineerObjectPacket* pkt = new EngineerObjectPacket{};
    pkt->type = PacketType::ENGINEER_OBJECT;
    pkt->hp = hp;
    pkt->ID = ID;
    
    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(EngineerObjectPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_CenterBuildingPacket(stClientInfo* receiver, int hp) {
    CenterBuildingPacket* pkt = new CenterBuildingPacket{};
    pkt->type = PacketType::CENTER_HP;
    pkt->damage = hp;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(CenterBuildingPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_GrenadePacket(stClientInfo* receiver, float posX,float posY,float posZ,float rotX,float rotY,float rotZ) {
    GrenadePacket* pkt = new GrenadePacket{};
    pkt->type = PacketType::GRENADE;
    pkt->posX = posX;
    pkt->posY = posY;
    pkt->posZ = posZ;
    pkt->rotX = rotX;
    pkt->rotY = rotY;
    pkt->rotZ = rotZ;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(GrenadePacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_PlayerArrivalPacket(stClientInfo* receiver, unsigned int playerID, bool arrive) {
    PlayerArrivalPacket* pkt = new PlayerArrivalPacket{};
    pkt->type = PacketType::PLAYER_ARRIVAL;
    pkt->playerID = playerID;
    pkt->arrive = arrive;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(PlayerArrivalPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
    };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_ClearCountPacket(stClientInfo* receiver, int PlayerCount) {
    ClearCountPacket* pkt = new ClearCountPacket{};
    pkt->type = PacketType::CLEAR_COUNT;
    pkt->PlayerCount = PlayerCount;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(ClearCountPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_ChooseJobPacket(stClientInfo* receiver, unsigned int playerID,int job) {
    ChooseJobPacket* pkt = new ChooseJobPacket{};
    pkt->type = PacketType::CHOOSE_JOB;
    pkt->playerID = playerID;
    pkt->job = job;
    std::cout << "SendData_ChooseJobPacket:P: " << pkt->playerID << "J: " << pkt->job << std::endl;
    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(ChooseJobPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}

void IOCompletionPort::SendData_ReadyPacket(stClientInfo* receiver,unsigned int id ) {
    ReadyPacket* pkt = new ReadyPacket{};
    pkt->type = PacketType::READY;
    pkt->playerID = id;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(ReadyPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_BangPacket(stClientInfo* receiver, unsigned int id) {
    BangPacket* pkt = new BangPacket{};
    pkt->type = PacketType::BANG;
    pkt->playerID = id;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(BangPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_PlayerDeathPacket(stClientInfo* receiver, unsigned int id) {
    PlayerDeathPacket* pkt = new PlayerDeathPacket{};
    pkt->type = PacketType::PLAYER_DEATH;
    pkt->playerID = id;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(PlayerDeathPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_DisconnectPacket(stClientInfo* receiver, unsigned int id) {
    DisconnectPacket* pkt = new DisconnectPacket{};
    pkt->type = PacketType::DISCONNECT;
    pkt->playerID = id;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(DisconnectPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    std::cout << "DISCONNECTpkt!!- "<< pkt->playerID << std::endl;
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        std::cout << "DISCONNECT패킷전송오류!" << std::endl;
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_MasterKeytPacket(stClientInfo* receiver, int key) {
    MasterKeyPacket* pkt = new MasterKeyPacket{};
    pkt->type = PacketType::MASTER_KEY;
    pkt->keyNum = key;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(MasterKeyPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        //std::cout << "DISCONNECT패킷전송오류!" << std::endl;
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_AttackObjectPacket(stClientInfo* receiver, int id,int damage) {
    AttackObjectPacket* pkt = new AttackObjectPacket{};
    pkt->type = PacketType::ATTACK_OBJECT;
    pkt->id = id;
    pkt->damage = damage;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(AttackObjectPacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        //std::cout << "DISCONNECT패킷전송오류!" << std::endl;
        delete pkt;
        delete sendOver;
    }
}
void IOCompletionPort::SendData_PlayerUpgradePacket(stClientInfo* receiver, int player_id, int random_id) {
    PlayerUpgradePacket* pkt = new PlayerUpgradePacket{};
    pkt->type = PacketType::UPGRADE;
    pkt->player_id = player_id;
    pkt->random_id = random_id;

    stOverlappedEx* sendOver = new stOverlappedEx{};
    ZeroMemory(&sendOver->overlapped, sizeof(sendOver->overlapped));
    sendOver->operation = IOOperation::SEND;
    sendOver->wsaBuf.buf = reinterpret_cast<char*>(pkt);
    sendOver->wsaBuf.len = sizeof(PlayerUpgradePacket);
    sendOver->cleanup = [pkt, sendOver]() {
        delete pkt;
        delete sendOver;
        };
    int ret = WSASend(receiver->socketClient, &sendOver->wsaBuf, 1, nullptr, 0,
        &sendOver->overlapped,
        NULL);
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        closesocket(receiver->socketClient);
        PostRemove(receiver);
        //std::cout << "DISCONNECT패킷전송오류!" << std::endl;
        delete pkt;
        delete sendOver;
    }
}

float CalcDistance3D(const XMFLOAT3& A, const XMFLOAT3& B) {
    XMVECTOR VecA = XMLoadFloat3(&A);
    XMVECTOR VecB = XMLoadFloat3(&B);
    XMVECTOR Diff = XMVectorSubtract(VecA, VecB);
    XMVECTOR Length = XMVector3Length(Diff);
    return XMVectorGetX(Length);
}
void IOCompletionPort::WorkThread() {
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    OVERLAPPED* overlapped;
    int room_id = 0;
    std::chrono::steady_clock::time_point Ltime = std::chrono::steady_clock::now();
    while (isRunning) {
        BOOL result = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE);
        stOverlappedEx* pOverlappedEx = reinterpret_cast<stOverlappedEx*>(overlapped);
        if (pOverlappedEx->operation == IOOperation::ACCEPT) {
            stClientInfo* newClient = new stClientInfo();
            newClient->socketClient = pOverlappedEx->acceptSocket;

            setsockopt(newClient->socketClient, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                (char*)&listenSocket, sizeof(listenSocket));
            // 소켓 옵션 설정
            BOOL bNoDelay = TRUE;
            setsockopt(newClient->socketClient, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(BOOL));

            CreateIoCompletionPort((HANDLE)newClient->socketClient, iocpHandle, (ULONG_PTR)newClient, 0);

            if (idCount%3==0) {
                room_id++;
            }
            newClient->roomID = room_id;
            newClient->id = idCount;
            idCount++;//=0
            //clients.push_back(newClient);
            AddClient(newClient);
            // 기존 처리 함수 호출
            
            std::cout << "입장idcount:" << idCount << std::endl;
            std::cout << "clientCount:" << clientCount << std::endl;
            //CreateRoom(roomMembers);
            
            SendData_EnterRoom(newClient);
            //NotifyOthersAboutNewClient(newClient);
            //SendExistingClientsToNewClient(newClient);

            {
                std::lock_guard<std::mutex> lock(waitMutex);
                waitingClients.push_back(newClient);

               std::vector<stClientInfo*> validClients;
               for (auto* client : waitingClients) {
                   if (client && !client->alreadyRemoved)
                       validClients.push_back(client);
               }
               
               if (validClients.size() >= MIN_PLAYER_COUNT) {
                   std::vector<stClientInfo*> roomMembers(validClients.begin(), validClients.begin() + MIN_PLAYER_COUNT);
               
                   for (auto* c : roomMembers) {
                       waitingClients.erase(std::remove(waitingClients.begin(), waitingClients.end(), c), waitingClients.end());
                   }
               
                   CreateRoom(roomMembers);
               }

               
            }
            RegisterRecv(newClient);
            // 다음 AcceptEx 등록
            PostAccept();
            delete pOverlappedEx;
            ProcessDelayedRemoves();
            continue;
        }
        
        // -------------- [DISCONNECT 처리] --------------
        if (!result || bytesTransferred == 0)
        {
            if (completionKey == 9999 || completionKey == 0) {
                // 리스닝 소켓용 AcceptEx 실패·취소
                delete pOverlappedEx;          // <- 이때는 Accept overlapped라 delete OK
                PostAccept();
                continue;
            }

            stClientInfo* client = reinterpret_cast<stClientInfo*>(completionKey);

            bool found = std::any_of(clients.begin(), clients.end(),
                [&](auto* s) { return s == client; });
            if (!found)
            {
                // 이미 삭제된 포인터이거나 쓰레기 포인터 -> overlapped만 Accept일 가능성
                if (pOverlappedEx && pOverlappedEx->operation == IOOperation::ACCEPT)
                    delete pOverlappedEx;
                continue;
            }

            std::cerr << "[연결 종료] 클라이언트 ID "
                << client->id << " 접속 해제!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            //Remove--
             // 방 찾기
            // 3인이 모여 CreateRoom()이 돌기 전(대기열 상태)에 나간 클라이언트는
            // rooms에 아직 등록되어 있지 않다. 이때는 브로드캐스트만 건너뛰고
            // 세션 정리는 똑같이 수행해야 한다.
            // (여기서 return하면 유일한 워커 스레드가 종료되어 서버 전체가 멈춘다)
            auto it = rooms.find(client->roomID);
            if (it != rooms.end()) {
                Room& room = it->second;
                for (auto* otherClient : room.clients) {
                    if (!otherClient || otherClient == client) continue;
                    SendData_DisconnectPacket(otherClient, client->id);
                }
            }
            else {
                std::cout << "[연결 종료] roomID " << client->roomID
                    << " 미등록 - 대기열 상태 종료로 보고 세션만 정리\n";
            }
            PostRemove(client);
            ProcessDelayedRemoves();
            //RemoveClient(client);              // client와 함께 overlapped 메모리도 해제됨
            // !! 여기서 pOverlappedEx 는 client 내부 메모리 → 더 이상 delete 금지 !!
            continue;
        }
        // ----------------------------------------------

        if (pOverlappedEx->operation == IOOperation::RECV) {
            stClientInfo* client = reinterpret_cast<stClientInfo*>(completionKey);
            stOverlappedEx* overEx = reinterpret_cast<stOverlappedEx*>(overlapped);

            int totalSize = bytesTransferred + client->prevRemainSize;
            char* buffer = new char[totalSize];
            memcpy(buffer, client->prevBuffer, client->prevRemainSize);
            memcpy(buffer + client->prevRemainSize, overEx->buffer, bytesTransferred);

            int processed = 0;

            while (totalSize - processed >= sizeof(PacketType)) {
                PacketType* type = reinterpret_cast<PacketType*>(buffer + processed);
                int packetSize = GetPacketSizeByType(*type);

                if (packetSize <= 0 || totalSize - processed < packetSize)
                    break;

                ProcessPacket(buffer + processed, client);

                processed += packetSize;
            }

            client->prevRemainSize = totalSize - processed;
           //if (client->prevRemainSize > 0)
           //    memcpy(client->prevBuffer, buffer + processed, client->prevRemainSize);
            if (client->prevRemainSize > 0) {
                if (client->prevRemainSize > MAX_SOCKBUF) {
                    std::cout << "[WARN] Remaining buffer too large, discarding\n";
                    client->prevRemainSize = 0;
                }
                else {
                    memcpy(client->prevBuffer, buffer + processed, client->prevRemainSize);
                }
            }
            delete[] buffer;
            //auto it = rooms.find(client->roomID);
            //if (it == rooms.end()) {
            //    continue;
            //    std::cout << "continue\n";
            //}
            //Room& room = it->second;
           
            RegisterRecv(client);  // 다시 수신 대기
            if (pOverlappedEx->cleanup) {
                pOverlappedEx->cleanup();
            }
            else {
                delete pOverlappedEx;
            }
        }
        if (pOverlappedEx->operation == IOOperation::SEND) {
            if (pOverlappedEx->cleanup) {
                pOverlappedEx->cleanup();  // 💡 안전하게 패킷 + overlapped 메모리 해제
            }
            else {
                delete pOverlappedEx;
            }
            ProcessDelayedRemoves();
            continue;
        }
        ProcessDelayedRemoves();
    }
}
std::unordered_map<int, Room>& IOCompletionPort::Rooms() { return rooms; }
std::mutex& IOCompletionPort::RoomMapMutex() { return roomMapMutex; }
std::atomic_bool& IOCompletionPort::RunningFlag() { return isRunning; }

void IOCompletionPort::BroadcastStageTimer(unsigned roomID, int stage, int remainingSeconds, bool running) {
    auto it = rooms.find((int)roomID);
    if (it == rooms.end()) return;
    Room& room = it->second;

    for (auto* c : room.clients) {
        if (!c || c->alreadyRemoved || c->socketClient == INVALID_SOCKET) continue;

        auto* pkt = new StageTimerPacket{};
        pkt->roomID = roomID;
        pkt->stage = stage;
        pkt->remainingSeconds = remainingSeconds;
        pkt->running = running;

        auto* ov = new stOverlappedEx{};
        ZeroMemory(&ov->overlapped, sizeof(ov->overlapped));
        ov->operation = IOOperation::SEND;
        ov->wsaBuf.buf = reinterpret_cast<char*>(pkt);
        ov->wsaBuf.len = sizeof(StageTimerPacket);
        ov->cleanup = [pkt, ov]() { delete pkt; delete ov; };

        WSASend(c->socketClient, &ov->wsaBuf, 1, nullptr, 0, &ov->overlapped, NULL);
    }
}
int IOCompletionPort::GetPacketSizeByType(PacketType type) {
    using PT = PacketType;

    if (type == PT::CHAT) return sizeof(ChatPacket_CtoS);
    if (type == PT::MOVE) return sizeof(MovePacket_CtoS);
    if (type == PT::VIEW_ANGLE) return sizeof(ViewingAnglePacket_CtoS);
    if (type == PT::ENTER) return sizeof(EnterRoomPacket);
    if (type == PT::ANIMATION) return sizeof(AnimationPacket_CtoS);
    if (type == PT::MTOP_DAMAGE) return sizeof(MtoPDamagePacket);
    if (type == PT::PTOM_DAMAGE) return sizeof(PtoMDamagePacket);
    if (type == PT::ENGINEER_INSTALL) return sizeof(EngineerInstallPacket);
    if (type == PT::ENGINEER_OBJECT) return sizeof(EngineerObjectPacket);
    if (type == PT::CENTER_HP) return sizeof(CenterBuildingPacket);
    if (type == PT::GRENADE) return sizeof(GrenadePacket);
    if (type == PT::PLAYER_ARRIVAL) return sizeof(PlayerArrivalPacket);
    if (type == PT::CLEAR_COUNT) return sizeof(ClearCountPacket);
    if (type == PT::CHOOSE_JOB) return sizeof(ChooseJobPacket);
    if (type == PT::READY) return sizeof(ReadyPacket);

    // 
    if (type == PT::NEW_CLIENT) return sizeof(NewClientPacket);
    if (type == PT::EXISTING_CLIENTS) return sizeof(ExistingClientsDataPacket);
    if (type == PT::MONSTER_STATE) return sizeof(MonsterStatePacket_CtoS);  // 또는 _StoC?
    if (type == PT::MONSTER_MOVE) return sizeof(MonsterMovePacket);
    if (type == PT::RANDOM_POSITION) return sizeof(DefenseRandomPacket);
    if (type == PT::BANG) return sizeof(BangPacket);
    if (type == PT::PLAYER_DEATH) return sizeof(PlayerDeathPacket);
    if (type == PT::DISCONNECT) return sizeof(DisconnectPacket);
    if (type == PT::FILE_LOAD) return sizeof(FilePacket);
    if (type == PT::MASTER_KEY) return sizeof(MasterKeyPacket);
    if (type == PT::NAME) return sizeof(NamePacket);
    if (type == PT::ATTACK_OBJECT) return sizeof(AttackObjectPacket);
    if (type == PT::UPGRADE) return sizeof(PlayerUpgradePacket);
    return 0; // 알 수 없는 타입이면 0
}
void IOCompletionPort::ProcessPacket(char* buffer, stClientInfo* client) {
    PacketType* packetType = reinterpret_cast<PacketType*>(buffer);
    if (*packetType == PacketType::NAME) {
        NamePacket* pkt = reinterpret_cast<NamePacket*>(buffer);
        std::cout << "name:" << pkt->name << std::endl;
        memcpy(client->name, pkt->name, pkt->size);
        client->name_size = pkt->size;
        client->name[pkt->size] = '\0';
        NotifyOthersAboutNewClient(client);
        SendExistingClientsToNewClient(client);

    }
    // 방 찾기
    auto it = rooms.find(client->roomID);
    if (it == rooms.end()) {
        std::cout << "[ProcessPacket] Invalid roomID\n";
        return; // ✅ continue 대신 return 권장 (room 없으면 더 이상 처리 의미 없음)
    }
    Room& room = it->second;

    if (*packetType == PacketType::MOVE) {
        MovePacket_CtoS* movePacket = reinterpret_cast<MovePacket_CtoS*>(buffer);

        client->x = movePacket->x;
        client->y = movePacket->y;
        client->z = movePacket->z;

        //auto it = rooms.find(client->roomID);
        //if (it == rooms.end()) break;  // 해당 룸이 없다면 무시
        //Room& room = it->second;
        XMFLOAT3 arrivePosition{};
        if (room.stageState == 1) {
            //arrivePosition= XMFLOAT3(120.0f, 0.0f, 94.0f );
            arrivePosition = MAP1_DESTINATION;
        }
        else if (room.stageState == 2) {
            arrivePosition = MAP2_DESTINATION;
        }
        else if (room.stageState == 3) {
            arrivePosition = MAP3_DESTINATION;
        }

        {
            std::lock_guard<std::mutex> lock(*room.roomMutex);
            int arrivedCount = 0;

            for (auto* c : room.clients) {
                if (!c) continue;

                XMFLOAT3 playerPosition{ c->x, c->y, c->z };
                float distance = CalcDistance3D(playerPosition, arrivePosition);

                bool prev = c->curr;
                c->curr = (distance <= 40.0f);

                if (c->curr != prev) {
                    //SendData_PlayerArrivalPacket(c, c->id, c->curr);
                    c->prev = c->curr;
                }

                if (c->curr) {
                    arrivedCount++;
                    //SendData_ClearCountPacket(c, arrivedCount);
                }
            }

            // clearCount 갱신
            room.clearCount = arrivedCount;

           

            //여기 room.clearCount가 3이면 다음 스테이지?

           
            if (room.clearCount == MIN_PLAYER_COUNT- room.DeathCount) {

                for (auto* Client : room.clients) {

                    if (Client->job == MG_JOB_TYPE) {
                        Client->hp = CHARACTER_MG_HP;
                    }
                    else if (Client->job == DMR_JOB_TYPE) {
                        Client->hp = CHARACTER_DMR_HP;
                    }
                    else if (Client->job == ENG_JOB_TYPE) {
                        Client->hp = CHARACTER_ENG_HP;
                    }
                    Client->curr = false;
                    Client->prev = false;
                    SendData_ClearCountPacket(Client, arrivedCount);
                }
                std::cout << "다들어옴!\n";
                //room.defenseState = true;
                room.stageState++;
                if (timerService) timerService->stopRoom(room.roomID);
                room.centerHp = CENTER_HP;
                room.DeathCount = 0;
                room.defenseState = false;
                room.monsterRandomSent = false;//// ★ 종료 1회 알림 + 타이머 중단
                room.nextMonsterToSend = 0;
                room.clearCount = 0;
                //if (randomTreadFlag2) {
                //    randomPositionThread2 = std::thread([this]() { RandomPositionThread2(); });
                //    randomTreadFlag2 = false;
                //}
                //if (randomTreadFlag2 == false && randomTreadFlag3 == true && room.stageState > 2) {
                //    randomPositionThread3 = std::thread([this]() { RandomPositionThread3(); });
                //    randomTreadFlag3 = false;
                //}
            }
            // 모든 room 클라이언트에게 clearCount 전송
           //for (auto* c : room.clients) {
           //    if (c) SendData_ClearCountPacket(c, arrivedCount);
           //}

        }

        // 이동 패킷을 같은 방 클라이언트들에게만 전송
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_Move(client, otherClient);
        }
    }
    else if (*packetType == PacketType::VIEW_ANGLE) {

        ViewingAnglePacket_CtoS* pkt = reinterpret_cast<ViewingAnglePacket_CtoS*>(buffer);
        client->angle_x = pkt->x;
        client->angle_y = pkt->y;
        client->angle_z = pkt->z;

        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_ViewAngle(client, otherClient);
        }

    }
    else if (*packetType == PacketType::ANIMATION) {
        //if (bytesTransferred < sizeof(MovePacket)) {
        //    std::cerr << "[에러] MOVE 패킷 크기 오류: " << bytesTransferred << " bytes" << std::endl;
        //    continue;
        //}


        AnimationPacket_CtoS* pkt = reinterpret_cast<AnimationPacket_CtoS*>(buffer);
        client->animationType = pkt->anymationType;

        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_Animaion(client, otherClient);
        }
    }
    else if (*packetType == PacketType::MONSTER_STATE) {
        //if (bytesTransferred < sizeof(MovePacket)) {
        //    std::cerr << "[에러] MOVE 패킷 크기 오류: " << bytesTransferred << " bytes" << std::endl;
        //    continue;
        //}



        MonsterStatePacket_CtoS* pkt = reinterpret_cast<MonsterStatePacket_CtoS*>(buffer);
        std::cout << pkt->id << " --  Monstertype:" << pkt->Mtype << " monID:" << pkt->id << ", state:" << pkt->state << std::endl;
        std::lock_guard<std::mutex> lock(*room.roomMutex);
        if (room.defenseState) {
            if (room.stageState == 1) {
                room.defenseMonsters[pkt->id].state = pkt->state;
            }
            else if (room.stageState == 2) {
                room.defenseMonsters2[pkt->id].state = pkt->state;
            }
            else if (room.stageState == 3) {
                room.defenseMonsters3[pkt->id].state = pkt->state;
            }
        }
        else {
            if (room.stageState == 1) {
                room.myMonsters[pkt->id].state = pkt->state;
            }
            else if (room.stageState == 2) {
                room.myMonsters2[pkt->id].state = pkt->state;
            }
            else if (room.stageState == 3) {
                room.myMonsters3[pkt->id].state = pkt->state;
            }
        }

        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_MonsterState(otherClient, pkt->Mtype, pkt->state, pkt->id);
        }
    }
    else if (*packetType == PacketType::MONSTER_MOVE) {
        //if (bytesTransferred < sizeof(MovePacket)) {
        //    std::cerr << "[에러] MOVE 패킷 크기 오류: " << bytesTransferred << " bytes" << std::endl;
        //    continue;
        //}



        MonsterMovePacket* pkt = reinterpret_cast<MonsterMovePacket*>(buffer);
        //std::cout << "MonsterMovePacket: " << pkt->playerId << std::endl;
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_MonsterMove(otherClient, pkt->x, pkt->y, pkt->z, pkt->angle_y, pkt->monsterId, pkt->playerId);
        }
    }
    else if (*packetType == PacketType::PTOM_DAMAGE) {

        //std::cout << "[DEBUG] 받은 바이트 수: " << bytesTransferred << std::endl;
        PtoMDamagePacket* pkt = reinterpret_cast<PtoMDamagePacket*>(buffer);
        std::cout << "PtoMDamagePacket: " << pkt->attackHp << "->monID: " << pkt->monsterID << std::endl;
        int sendHP = 0;
        {
            std::lock_guard<std::mutex> lock(*room.roomMutex);
            if (room.defenseState) {
                if (room.stageState == 1) {
                    
                    auto& m = room.defenseMonsters[pkt->monsterID];
                    m.hp -= pkt->attackHp;
                    if (m.hp < 0) {
                        m.hp = 0;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                        }
                    }
                    sendHP = m.hp;
                    if (std::all_of(room.defenseMonsters.begin(), room.defenseMonsters.end(), [](const MonsterData& m) { return m.hp <= 0; })) {
                        room.defenseState = false;
                        //if (timerService) timerService->resetRoom(nextRoomID - 1, TIMER_ADVENTURE); // ★ 방 타이머 60초 시작
                    }
                    

                    std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                    for (auto* otherClient : room.clients) {
                        if (!otherClient) continue;
                        SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                    }
                }
                else if (room.stageState == 2) {
                    
                    auto& m = room.defenseMonsters2[pkt->monsterID];
                    m.hp -= pkt->attackHp;
                    if (m.hp < 0) {
                        m.hp = 0;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                        }
                    }
                    sendHP = m.hp;
                    if (std::all_of(room.defenseMonsters2.begin(), room.defenseMonsters2.end(), [](const MonsterData& m) { return m.hp <= 0; })) {
                        room.defenseState = false;
                        //if (timerService) timerService->resetRoom(nextRoomID - 1, TIMER_ADVENTURE); // ★ 방 타이머 60초 시작
                    }
                    

                    std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                    for (auto* otherClient : room.clients) {
                        if (!otherClient) continue;
                        SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                    }
                }
                else if (room.stageState == 3) {
                   
                    auto& m = room.defenseMonsters3[pkt->monsterID];
                    m.hp -= pkt->attackHp;
                    if (m.hp < 0) {
                        m.hp = 0;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                        }
                    }
                    sendHP = m.hp;
                    if (std::all_of(room.defenseMonsters3.begin(), room.defenseMonsters3.end(), [](const MonsterData& m) { return m.hp <= 0; })) {
                        room.defenseState = false;
                        //if (timerService) timerService->resetRoom(nextRoomID - 1, TIMER_ADVENTURE); // ★ 방 타이머 60초 시작
                    }
                    

                    std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                    for (auto* otherClient : room.clients) {
                        if (!otherClient) continue;
                        SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                    }

                }
            }
            else {
                if (pkt->monsterID <= 1000) {
                    if (room.stageState == 1) {
                        auto& m = room.myMonsters[pkt->monsterID];
                        m.hp -= pkt->attackHp;
                        if (m.hp < 0) {
                            m.hp = 0;
                            for (auto* otherClient : room.clients) {
                                if (!otherClient) continue;
                                SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                            }
                        }
                        sendHP = m.hp;
                        std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                        }
                    }
                    else if (room.stageState == 2) {
                        auto& m = room.myMonsters2[pkt->monsterID];
                        m.hp -= pkt->attackHp;
                        if (m.hp < 0) {
                            m.hp = 0;
                            for (auto* otherClient : room.clients) {
                                if (!otherClient) continue;
                                SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                            }
                        }
                        sendHP = m.hp;
                        std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                        }
                    }
                    else if (room.stageState == 3) {
                        auto& m = room.myMonsters3[pkt->monsterID];
                        m.hp -= pkt->attackHp;
                        if (m.hp < 0) {
                            m.hp = 0;
                            for (auto* otherClient : room.clients) {
                                if (!otherClient) continue;
                                SendData_MonsterState(otherClient, 0, 3, pkt->monsterID);
                            }
                        }
                        sendHP = m.hp;
                        std::cout << pkt->monsterID << "번---PTOM_DAMAGE HP: " << sendHP << std::endl;
                        for (auto* otherClient : room.clients) {
                            if (!otherClient) continue;
                            SendData_PtoMDamagePacket(otherClient, pkt->monsterID, sendHP);
                        }
                    }
                }
            }
        }

    }

    else if (*packetType == PacketType::MTOP_DAMAGE) {


        MtoPDamagePacket* pkt = reinterpret_cast<MtoPDamagePacket*>(buffer);
        std::cout << "MTOP_DAMAGE: 맞은 데미지-" << pkt->attackHp << std::endl;
        int myHP = 0;
        {
            std::lock_guard<std::mutex> lock(*room.roomMutex);
            for (auto* targetClient : room.clients) {
                if (!targetClient) continue;
                if (targetClient->id == pkt->playerID) {
                    targetClient->hp -= pkt->attackHp;
                    if (targetClient->hp < 0) { 
                        targetClient->hp = 0; 
                        room.DeathCount++;
                        //SendData_PlayerDeathPacket(targetClient, targetClient);
                    }
                    if (targetClient->job == MG_JOB_TYPE) {
                        if (targetClient->hp > CHARACTER_MG_HP) targetClient->hp = CHARACTER_MG_HP;
                    }
                    else if (targetClient->job == DMR_JOB_TYPE) {
                        if (targetClient->hp > CHARACTER_DMR_HP) targetClient->hp = CHARACTER_DMR_HP;
                    }
                    else if (targetClient->job == ENG_JOB_TYPE) {
                        if (targetClient->hp > CHARACTER_ENG_HP) targetClient->hp = CHARACTER_ENG_HP;
                    }
                    myHP = targetClient->hp;
                    break;
                }

            }

            if (client->hp == 0) {
                for (auto* c : room.clients) {
                    if (!c) continue;
                    SendData_PlayerDeathPacket(c, client->id);
                    std::cout << "데스패킷전송\n";
                }
            }
           
            
        }
        for (auto* otherClient : room.clients) {
            if (!otherClient) continue;
            SendData_MtoPDamagePacket(otherClient, pkt->playerID, pkt->monsterID, myHP);
        }

    }
    else if (*packetType == PacketType::ENGINEER_INSTALL) {


        EngineerInstallPacket* pkt = reinterpret_cast<EngineerInstallPacket*>(buffer);
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_EngineerInstallPacket(otherClient, pkt->Etype, pkt->ID, pkt->rotY, pkt->posX, pkt->posY, pkt->posZ);
        }
    }
    else if (*packetType == PacketType::ENGINEER_OBJECT) {


        EngineerObjectPacket* pkt = reinterpret_cast<EngineerObjectPacket*>(buffer);
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_EngineerObjectPacket(otherClient, pkt->ID, pkt->hp);
        }
    }
    else if (*packetType == PacketType::CENTER_HP) {


        CenterBuildingPacket* pkt = reinterpret_cast<CenterBuildingPacket*>(buffer);
        {
            std::lock_guard<std::mutex> lock(*room.roomMutex);
            room.centerHp -= pkt->damage;
            if (room.centerHp < 0) room.centerHp = 0;
        }
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_CenterBuildingPacket(otherClient, room.centerHp);
        }
    }
    else if (*packetType == PacketType::GRENADE) {

        //GrenadePacket* pkt = reinterpret_cast<GrenadePacket*>(pOverlappedEx->buffer);
        //
        //// 데미지 패킷을 모든 클라이언트에게 전송
        //for (stClientInfo* otherClient : clients) {
        //    if (!otherClient) continue;
        //    if (otherClient != client /*&& client->roomID == otherClient->roomID*/) { // 패킷을 보낸 클라이언트에게는 다시 전송하지 않음
        //        SendData_GrenadePacket(otherClient, pkt->posX, pkt->posY, pkt->posZ, pkt->rotX, pkt->rotY, pkt->rotZ);
        //    }
        //}
        GrenadePacket* pkt = reinterpret_cast<GrenadePacket*>(buffer);
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_GrenadePacket(otherClient, pkt->posX, pkt->posY, pkt->posZ, pkt->rotX, pkt->rotY, pkt->rotZ);
        }
    }
    else if (*packetType == PacketType::CHOOSE_JOB) {

        ChooseJobPacket* pkt = reinterpret_cast<ChooseJobPacket*>(buffer);
        //std::cout << "CHOOSE_JOB--id:" << pkt->playerID << "job:" << pkt->job << std::endl;
        client->job = pkt->job;

        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            std::cout << "INroomPlayer: " << otherClient->id << std::endl;
            SendData_ChooseJobPacket(otherClient, pkt->playerID, pkt->job);
        }
    }

    else if (*packetType == PacketType::READY) {

        ReadyPacket* pkt = reinterpret_cast<ReadyPacket*>(buffer);
        //std::cout << "CHOOSE_JOB--id:" << pkt->playerID << "job:" << pkt->job << std::endl;
        client->ready = true;


        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_ReadyPacket(otherClient, pkt->playerID);
        }
        bool allReady = true;
        for (auto* otherClient : room.clients) {
            if (otherClient->ready == false) {
                allReady = false;
            }
        }
        if (allReady) {
            //randomthread
            for (auto* Client : room.clients) {

                if (Client->job == MG_JOB_TYPE) {
                    Client->hp = CHARACTER_MG_HP;
                }
                else if (Client->job == DMR_JOB_TYPE) {
                    Client->hp = CHARACTER_DMR_HP;
                }
                else if (Client->job == ENG_JOB_TYPE) {
                    Client->hp = CHARACTER_ENG_HP;
                }
            }
            std::cout << "all players ready!!!\n";
        }
    }
    else if (*packetType == PacketType::BANG) {

        BangPacket* pkt = reinterpret_cast<BangPacket*>(buffer);
        std::cout << "BANG--id:" << pkt->playerID << std::endl;
        
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_BangPacket(otherClient, pkt->playerID);
        }

    }
    else if (*packetType == PacketType::FILE_LOAD) {

        FilePacket* pkt = reinterpret_cast<FilePacket*>(buffer);
        std::cout << "FILE_LOAD--stage:"<< pkt->stage << std::endl;
        room.fileCount++;
        if (pkt->startDefense) {
            if (room.fileCount >= MIN_PLAYER_COUNT) {
                room.defenseState = true;
                room.fileCount = 0;
            }
        }
        else {
            if (room.fileCount >= MIN_PLAYER_COUNT) {
                if (timerService) timerService->resetRoom(nextRoomID - 1, TIMER_ADVENTURE);
                room.fileCount = 0;
            }
        }

       
        //if (pkt->stage == 1) {
        //    if (randomTreadFlag1) {
        //        randomPositionThread = std::thread([this]() { RandomPositionThread(); });
        //        randomTreadFlag1 = false;
        //    }
        //}
        //else if (pkt->stage == 2) {
        //    if (randomTreadFlag2) {
        //        randomPositionThread2= std::thread([this]() { RandomPositionThread2(); });
        //        randomTreadFlag2 = false;
        //    }
        //}
        //else if (pkt->stage == 3) {
        //    if (randomTreadFlag3) {
        //        randomPositionThread3 = std::thread([this]() { RandomPositionThread3(); });
        //        randomTreadFlag3 = false;
        //    }
        //}
        
    }
    else if (*packetType == PacketType::ATTACK_OBJECT) {

        AttackObjectPacket* pkt = reinterpret_cast<AttackObjectPacket*>(buffer);
        for (auto* otherClient : room.clients) {
            if (!otherClient) continue;
            SendData_AttackObjectPacket(otherClient, pkt->id,pkt->damage);
        }

    }
    else if (*packetType == PacketType::UPGRADE) {

        PlayerUpgradePacket* pkt = reinterpret_cast<PlayerUpgradePacket*>(buffer);
        for (auto* otherClient : room.clients) {
            if (!otherClient || otherClient == client) continue;
            SendData_PlayerUpgradePacket(otherClient, pkt->player_id, pkt->random_id);
        }

    }
    else if (*packetType == PacketType::MASTER_KEY) {
       //MasterKeyPacket* pkt = reinterpret_cast<MasterKeyPacket*>(buffer);
       //if (pkt->keyNum == 1) {
       //    client->x = MAP1_DESTINATION.x;
       //    client->y = MAP1_DESTINATION.y;
       //    client->z = MAP1_DESTINATION.z;
       //    for (auto* otherClient : room.clients) {
       //        if (!otherClient || otherClient == client) continue;
       //        SendData_Move(client, otherClient);
       //    }
       //}
       //if (pkt->keyNum == 2) {
       //    client->x = MAP2_DESTINATION.x;
       //    client->y = MAP2_DESTINATION.y;
       //    client->z = MAP2_DESTINATION.z;
       //    for (auto* otherClient : room.clients) {
       //        if (!otherClient || otherClient == client) continue;
       //        SendData_Move(client, otherClient);
       //    }
       //}
       //if (pkt->keyNum == 3) {
       //    client->x = MAP3_DESTINATION.x;
       //    client->y = MAP3_DESTINATION.y;
       //    client->z = MAP3_DESTINATION.z;
       //    for (auto* otherClient : room.clients) {
       //        if (!otherClient || otherClient == client) continue;
       //        SendData_Move(client, otherClient);
       //    }
       //}

        //for (auto* otherClient : room.clients) {
        //    if (!otherClient || otherClient == client) continue;
        //    SendData_MasterKeytPacket(otherClient, pkt->keyNum);
        //}
    }

}


void IOCompletionPort::DestroyThread() {
    isRunning = false;
    CloseHandle(iocpHandle);
    closesocket(listenSocket);

    //for (auto& th : workerThreads) {
    //    if (th.joinable()) th.join();
    //} 
    if (workerThread.joinable()) workerThread.join();

    //if (accepterThread.joinable()) accepterThread.join();
   // if (npcThread.joinable()) npcThread.join();
    if (randomPositionThread.joinable()) randomPositionThread.join();
    if (randomPositionThread2.joinable()) randomPositionThread2.join();
    if (randomPositionThread3.joinable()) randomPositionThread3.join();
    if (timerService) { timerService->stop(); timerService.reset(); }
    std::cout << "서버 종료 완료.\n";
}
