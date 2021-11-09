"""Made by Charlie"""
import csv
import pygame
import os
from io import BytesIO
from PIL import Image

pygame.init()

left, right = 1, 3
break_every_sentence = True  # 开启分句渲染
display_speaker_name = True  # 是否在左上方显示说话者
preview_while_rendering = False  # 自动打开图片
input_file_name = "test.csv"  # 输入文件名
font_name = "SourceHanSansSC-Medium.otf"  # 字体文件名，需放在 /fonts 目录下
font_size_CHN = 40  # 中文字体大小（勿动）
font_size_ENG = 24  # 英文字体大小（勿动）
character_setting = {  # 人物字体颜色设置
    "Christine": {
        "color": (255, 88, 73)
    },  # rgb(255, 88, 73)
    "Carlotta": {
        "color": (83, 25, 230)
    },  # rgb(83, 25, 230)
    "Phantom": {
        "color": (0, 44, 69)
    },  # rgb(0, 44, 69)
    "Giry": {
        "color": (255, 121, 150)
    },  # rgb(255, 121, 150)
    "Meg": {
        "color": (123, 195, 0)
    },  # rgb(123, 195, 0)
    "Raoul": {
        "color": (0, 120, 200)
    },  # rgb(0, 120, 200)
    "绅士": {
        "color": (44, 94, 144)
    },  # rgb(44, 94, 144)
    "Singing": {
        "color": (44, 44, 44)
    },  # rgb(44, 44, 44)
}


def write(
        img,
        text,
        linenum,
        fontsize=32,
        color=(0, 0, 0),
        bgcolor=(255, 255, 255),
        align=True,
):
    font = pygame.font.Font(os.path.join("fonts", font_name), fontsize)
    rtext = font.render(text, True, color, bgcolor)
    l, h = rtext.get_size()
    buffer = BytesIO()
    pygame.image.save(rtext, buffer)
    if align:
        length = int(960 - l / 2)
    else:
        length = 500
    height = 86 * int(linenum / 2)
    if fontsize == font_size_ENG:
        height -= 34
    img.paste(Image.open(buffer), (length, height + 5))


def render(id, text):
    if not left <= id <= right:
        return
    img = Image.new("RGBA", (1920, 1080))
    n = len(text)
    chara = []
    con = ""
    for i in range(0, len(text)):
        chara.append(text[i][0])
        try:
            c = character_setting[text[i][0]]["color"]
        except KeyError:
            print('[WARN] (line {0}) 未找到角色 "{1}", 使用默认颜色'.format(
                id, text[i][0]))
            c = (0, 0, 0)
        write(
            img,
            text[i][1],
            2 * i + 2,
            fontsize=font_size_ENG,
            color=c,
        )
        if text[i][0] != "" and display_speaker_name and text[i][0] != con:
            write(
                img,
                "[{0}] {1}".format(text[i][0], text[i][2]),
                2 * i + 1,
                fontsize=font_size_CHN,
                color=c,
            )
            # write(
            #     img,
            #     "[{0}]".format(text[i][0]),
            #     2 * i + 1,
            #     fontsize=font_size_CHN,
            #     color=c,
            #     align=False,
            # )
        else:
            write(
                img,
                text[i][2],
                2 * i + 1,
                fontsize=font_size_CHN,
                color=c,
            )
        con = text[i][0]
        if break_every_sentence:
            img.save("{0}-({1}-{2}).png".format(id, i, n))
    print("[Render] (line {0}) [{1}] Character: {2}".format(id, n, chara))
    if not break_every_sentence:
        img.save("{0}.png".format(id))
    if preview_while_rendering:
        img.show()


def isDiff(a, b, s):
    return a == s and b != s or a != s and b == s


with open(input_file_name, encoding="utf-8") as f:
    reader = csv.reader(f, delimiter=";")
    i = 0
    id = 0
    a = []
    b = []
    c = []
    while True:
        try:
            a.append(next(reader))
        except StopIteration:
            break
        except Exception as e:
            print("[ERROR] {0}".format(e))
    for sent in a:
        length = int((len(sent) - 1) / 2)
        for i in range(0, length):
            b.append((sent[0], sent[i * 2 + 1], sent[i * 2 + 2]))
    c.append(b[0])
    for ind in range(0, len(b) - 1):
        if i >= 3 or isDiff(b[ind][0], b[ind + 1][0], "Singing"):
            i = 0
            c.append(b[ind])
            id += 1
            render(id, c)
            c.clear()
        else:
            i += 1
            c.append(b[ind])
    if i != 0:
        id += 1
        c.append(b[-1])
        render(id, c)

    print("===== Finished! =====")
