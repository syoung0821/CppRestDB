// fDump.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace fs = std::filesystem;

// 설정 파일 경로
const std::string CONFIG_FILE_PATH = "c:\\Worklist\\server\\configuration.ini";

// 폴더 생성 함수
void createDumpFolder(const std::string& folderPath) {
    if (!fs::exists(folderPath)) {
        // create_directories는 상위 폴더까지 모두 생성함
        fs::create_directories(folderPath);
        std::cout << "폴더가 생성되었습니다: " << folderPath << std::endl;
    }
    else {
        std::cout << "폴더가 이미 존재합니다: " << folderPath << std::endl;
    }
}

// 외부 명령 실행 함수 (검사 추가)
void addExam(const std::string& dumpFolder, const std::string& examName) {
    std::string command = "dump2dcm -g " + dumpFolder + "\\" + examName + ".dump " + dumpFolder + "\\" + examName + ".wl";
    system(command.c_str());
    std::cout << examName << ".wl 파일이 생성되었습니다." << std::endl;
}

// 검사 삭제 함수
void deleteExam(const std::string& dumpFolder, const std::string& examName) {
    std::string filePath = dumpFolder + "\\" + examName + ".wl";
    if (fs::exists(filePath)) {
        fs::remove(filePath);
        std::cout << filePath << " 파일이 삭제되었습니다." << std::endl;
    }
    else {
        std::cout << filePath << " 파일이 존재하지 않습니다." << std::endl;
    }
}

// 검사 전체 삭제 함수
void deleteAllExams(const std::string& dumpFolder) {
    for (const auto& file : fs::directory_iterator(dumpFolder)) {
        fs::remove(file.path());
    }
    std::cout << dumpFolder << " 폴더의 모든 파일이 삭제되었습니다." << std::endl;
}

// 설정 파일 작성 함수
void createConfigFile() {
    std::ofstream configFile(CONFIG_FILE_PATH);
    if (configFile.is_open()) {
        configFile << "[Settings]\n";
        configFile << "AutoDelete=true\n";
        configFile << "DeleteTime=00:00\n"; // AM 12:00 이전 데이터 삭제
        configFile.close();
        std::cout << "설정 파일이 작성되었습니다: " << CONFIG_FILE_PATH << std::endl;
    }
}

// 설정 파일 읽기 및 자동 삭제 기능
void autoDeleteFiles(const std::string& dumpFolder) {
    std::ifstream configFile(CONFIG_FILE_PATH);
    if (configFile.is_open()) {
        std::string line;
        bool autoDelete = false;
        std::string deleteTime;

        // 설정 파일에서 값을 읽음
        while (std::getline(configFile, line)) {
            if (line.find("AutoDelete") != std::string::npos) {
                autoDelete = (line.find("true") != std::string::npos);
            }
            if (line.find("DeleteTime") != std::string::npos) {
                deleteTime = line.substr(line.find("=") + 1);
            }
        }
        configFile.close();

        // 자동 삭제 설정이 활성화된 경우
        if (autoDelete) {
            time_t now = time(0);
            tm localTime;

            // localtime_s는 첫 번째 인자로 결과를 저장할 구조체 포인터, 두 번째 인자로 시간을 받습니다.
            localtime_s(&localTime, &now);

            // 현재 시간이 설정된 삭제 시간보다 이전인 경우
            if (localTime.tm_hour == 0 && localTime.tm_min == 0) {
                std::cout << "자동 삭제 시간이 되었습니다." << std::endl;
                deleteAllExams(dumpFolder);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "사용법: " << argv[0] << " <경로> <폴더명>" << std::endl;
        return 1;
    }

    std::string basePath = argv[1];
    std::string dumpFolder = argv[2];

    // 2. 전달받은 경로에 dump 폴더 생성
    createDumpFolder(dumpFolder);

    // 3. 검사 추가
    addExam(dumpFolder, "worklist");

    // 4. 특정 검사 삭제
    deleteExam(dumpFolder, "worklist");

    // 5. 검사 전체 삭제
    deleteAllExams(dumpFolder);

    // 6. 설정 파일 작성
    createConfigFile();

    // 7. 자동 삭제 기능 실행
    autoDeleteFiles(dumpFolder);

    return 0;
}