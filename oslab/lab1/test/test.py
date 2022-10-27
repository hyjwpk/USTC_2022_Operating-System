import csv
import time
import random
from multiprocessing import Process, Value


def countdown(score, timeend):
    for timesec in range(120, 0, -1):
        print("    还剩：%3s秒" % timesec, end='\r')
        time.sleep(1)

    print("时间已到。得分：%d/4        " % score.value)
    timeend.value = 1
    return


def random_select(qlist, num):
    output = []
    qnumbers = random.sample(range(0, len(qlist)), num)

    for qnum in qnumbers:
        q = qlist[qnum][0]
        ans = qlist[qnum][1]
        selects = random.sample(qlist[qnum][2:], 3)
        selects.append(ans)
        random.shuffle(selects)

        output.append({'q': q, 'ans': ans, 'selects': selects})
    return output


def generate():
    # generate first three questions
    qlist = []
    with open('command.csv', encoding='UTF-8') as fin:
        reader = csv.reader(fin)
        for row in reader:
            qlist.append(row)
    with open('command-reverse.csv', encoding='UTF-8') as fin:
        reader = csv.reader(fin)
        for row in reader:
            qlist.append(row)

    output = random_select(qlist, 2)

    # generate last two questions
    qlist = []
    with open('questions.csv', encoding='UTF-8') as fin:
        reader = csv.reader(fin)
        for row in reader:
            qlist.append(row)

    return output + random_select(qlist, 2)


def start(questions, score, timeend):
    answered = [0, 0, 0, 0]
    while answered != [1, 1, 1, 1]:
        for i in range(len(questions)):
            q = questions[i]
            if answered[i] == 1:
                continue

            print("第%1d题：%s" % (i + 1, q['q']))
            for j in range(4):
                print('%1d: %s' % (j + 1, q['selects'][j]))
            print('')

            if timeend.value == 1:
                exit()
            num = input()
            if timeend.value == 1:
                exit()

            try:
                num = int(num)
                if num <= 0:
                    raise ValueError

                if q['selects'][num - 1] == q['ans']:
                    score.value += 1
                    print('正确！\n')
                else:
                    print('错误!\n')

            except (ValueError, IndexError):
                print("本题跳过。\n\n")
                continue
            answered[i] = True

    print("得分：%d/4" % score.value)


def main():
    score = Value("d", 0)
    timeend = Value("d", 0)
    try:
        questions = generate()
    except FileNotFoundError:
        print('找不到题库或题库不完整。请确认三个csv文件是否都在终端的工作目录下。')
        exit(-1)
    cdprocess = Process(target=countdown, args=(score, timeend, ))
    print("题目已生成完毕，按回车键开始答题。")
    print("答题时请输入数字1234作为选项，其他非法输入视为暂时跳过等会再答：")
    input()

    cdprocess.start()
    start(questions, score, timeend)
    cdprocess.kill()
    print("按回车键退出。")
    input()


if __name__ == "__main__":
    main()
