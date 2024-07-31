#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <omp.h>
#include <ctime>

// 定义棋盘大小
const int BOARD_SIZE = 81;
const int AI = 1;
const int OP = 2;
bool air_vis[81];
int winner;

// TrainData结构体定义
struct TrainData {
    float input[6 * BOARD_SIZE] = { 0 }; // 6个通道的输入
    float output_policy[BOARD_SIZE] = { 0 }; // 策略输出
    int output_value = 0; // 胜负值
};

// 移动位置
int moveTo(int p, int dir) {
    switch (dir) {
    case 0:
        return (p += 9) < 81 ? p : -1;
    case 1:
        return (p -= 9) >= 0 ? p : -1;
    case 2:
        return p % 9 < 8 ? p + 1 : -1;
    case 3:
        return p % 9 > 0 ? p - 1 : -1;
    }
    return p;
}

// 判断是否有气
bool hasAir(int m_board[], int p) {
    air_vis[p] = true;
    bool flag = false;
    for (int dir = 0; dir < 4; dir++) {
        int dp = moveTo(p, dir);
        if (dp >= 0) {
            if (m_board[dp] == 0)
                flag = true;
            if (m_board[dp] == m_board[p] && !air_vis[dp])
                if (hasAir(m_board, dp))
                    flag = true;
        }
    }
    return flag;
}

bool judgeAvailable(int m_board[], int p, int col) {
    if (m_board[p])
        return false;
    m_board[p] = col;
    memset(air_vis, 0, sizeof(air_vis));
    if (!hasAir(m_board, p)) {
        m_board[p] = 0;
        return false;
    }
    for (int dir = 0; dir < 4; dir++) {
        int dp = moveTo(p, dir);
        if (dp >= 0) {
            if (m_board[dp] && !air_vis[dp])
                if (!hasAir(m_board, dp)) {
                    m_board[p] = 0;
                    return false;
                }
        }
    }
    m_board[p] = 0;
    return true;
}

std::vector<TrainData> trainDatas;
void writeTrainDataToFile(const std::string& file) {
    std::ofstream outfile(file);
    for (const auto& item : trainDatas) {
        for (int i = 0; i < 6 * BOARD_SIZE; i++)
            outfile << item.input[i] << " ";
        for (int i = 0; i < BOARD_SIZE; i++)
            outfile << item.output_policy[i] << " ";
        outfile << item.output_value << std::endl;
    }
    outfile.close();
}

// makeTrainData函数定义
TrainData makeTrainData(int* board, int p, std::vector<float> probs, int last_p, bool first) {
    TrainData res;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i] == AI)
            res.input[0 * BOARD_SIZE + i] = 1; // 第一通道为我方棋子
        if (board[i] == OP)
            res.input[1 * BOARD_SIZE + i] = 1; // 第二通道为对方棋子
        if (judgeAvailable(board, i, AI))
            res.input[2 * BOARD_SIZE + i] = 1; // 第三通道为我方允许落子位置
        if (judgeAvailable(board, i, OP))
            res.input[3 * BOARD_SIZE + i] = 1; // 第四通道为对方允许落子位置
        if (first)
            res.input[4 * BOARD_SIZE + i] = 1; // 第五通道为我方是否先手
        res.output_policy[i] = probs[i];
    }
    if (last_p >= 0 && last_p < BOARD_SIZE)
        res.input[5 * BOARD_SIZE + last_p] = 1; // 第六通道为对方最后一手位置
    return res;
}

// 解析每行数据并调用makeTrainData函数
void processFile(const std::string& filename) {
    int count = 0;
    std::ifstream file(filename);
    std::string line;
    std::vector<int> board(BOARD_SIZE);
    std::vector<float> probs(BOARD_SIZE, 0.0f);
    // 获取胜者
    std::getline(file, line);
    std::istringstream iss(line);
    iss >> winner;
    bool flag;
    //黑色胜利-先1后-1
    if (winner == 1)flag = true;
    //白色胜利-先-1后1
    else if (winner == 2)flag = false;
    while (std::getline(file, line)) {
        int score;
        if (flag)score = 1;
        else score = -1;
        flag = !flag;
        std::istringstream iss(line);
        int pos, last_p;
        // 读取棋盘状态
        for (int i = 0; i < BOARD_SIZE; i++) {
            iss >> pos;
            board[i] = pos;
        }

        // 读取最后数字
        iss >> last_p;

        // 设置probs数组
        probs.assign(BOARD_SIZE, 0.0f);
        if (last_p >= 0 && last_p < BOARD_SIZE)
            probs[last_p] = 1.0f;

        // 判断是否为先手
        bool first = (board[0] == AI);

        // 调用makeTrainData函数
        TrainData data = makeTrainData(board.data(), winner, probs, last_p, first);
        data.output_value = score;

        // 保存数据
        trainDatas.push_back(data);
        count++;
        std::cout << count << "th data has been generated!" << std::endl;
    }
}

void processDirectory(const std::string& directoryPath) {
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".txt") {
            // 清空旧数据
            trainDatas.clear();

            std::string inputFilename = entry.path().string();
            std::string outputFilename = "./neural_data/"+ entry.path().stem().string() + "_output.txt";
            processFile(inputFilename);
            // 将数据写入文件
            printf("One file done~\n");
            printf("Saving......\n");
            writeTrainDataToFile(outputFilename);
        }
    }
    // 输出生成的trainDatas的数量
    std::cout << "Total TrainData objects generated: " << trainDatas.size() << std::endl;
}

int main() {
    std::string directoryPath = "./extracted_data";
    processDirectory(directoryPath);
    return 0;
}
