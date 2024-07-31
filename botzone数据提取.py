import json
import os
import time


def process_data(data):
    # 提取玩家信息
    players = data['players']
    player1 = players[0].get('bot')
    player2 = players[1].get('bot')
    if player1 is None or player2 is None:
        return 0, 0, 0, []

    # 提取分数
    scores = data['scores']

    # 提取日志信息
    log = data['log']

    # 处理日志数据
    moves = []
    for entry in log:
        if '0' in entry:
            player_move = entry['0'].get('response')
            if isinstance(player_move, dict) and len(player_move) == 2 and 'x' in player_move and 'y' in player_move:
                moves.append({'player': '0', 'x': player_move['x'], 'y': player_move['y']})
            else:
                break
        if '1' in entry:
            player_move = entry['1'].get('response')
            if isinstance(player_move, dict) and len(player_move) == 2 and 'x' in player_move and 'y' in player_move:
                moves.append({'player': '1', 'x': player_move['x'], 'y': player_move['y']})
            else:
                break

    return player1, player2, scores, moves


def update_board(board, move):
    index = move['y'] * 9 + move['x']
    if index < 0 or index > 81:
        return board
    board[index] = 1 if move['player'] == '0' else 2
    return board


folder_path = 'C:\\Users\\23375\\Desktop\\AlphaPig-NoGo-main\\NoGo-2024-6'
output_file_path = ''


for filename in os.listdir(folder_path):
    file_name = time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
    output_file_path = f'./extracted_data/{file_name}.txt'
    print('done&start!')
    with open(output_file_path, 'w') as output_file:
        # 只处理 .matches 文件
        if filename.endswith('.matches'):
            file_path = os.path.join(folder_path, filename)
            with open(file_path, 'r', encoding='utf-8') as file:
                # 读取文件中的每一行
                for line in file:
                    try:
                        # 尝试解析每一行的 JSON 数据
                        data = json.loads(line.strip())
                        player1, player2, scores, moves = process_data(data)
                        if player1 == 0 and player2 == 0:
                            continue

                        # 添加胜者信息
                        winner = '1' if scores[0] > scores[1] else '2'
                        output_file.write(f'{winner}\n')

                        # 初始化棋盘
                        board = [0] * 81
                        for move in moves:
                            x=move['x']
                            y=move['y']
                            last_p=x+y*9
                            board = update_board(board, move)
                            # 保存当前棋盘状态到文件
                            output_file.write(' '.join(map(str, board)))
                            output_file.write(f' {last_p}' + '\n')

                        output_file.write('\n')  # 每局棋盘状态之间的间隔

                    except json.JSONDecodeError as e:
                        print(f"Error decoding JSON from file {filename}, line: {line.strip()}\n{e}")
