"""Made by Charlie"""
import csv
import pygame
import os
from io import BytesIO
from PIL import Image

pygame.init()

从哪行开始, 到哪行结束 = 1, 3
break_every_sentence = False  # 开启分句渲染
display_speaker_name = True  # 是否在左上方显示说话者
AutoPreviewImage = True # 自动预览图片
input_file_name = "abc.csv"  # 输入文件名
font_name = "SourceHanSansSC-Medium.otf"  # 字体文件名，需放在 /fonts 目录下
font_size_CHN = 40  # 中文字体大小（勿动）
font_size_ENG = 24  # 英文字体大小（勿动）
character_setting = {  # 人物字体颜色设置
    "女一号": {"color": (255, 88, 73)},  # rgb(255, 88, 73)
    "男一号": {"color": (123, 195, 0)},  # rgb(123, 195, 0)
    "Raoul": {"color": (0, 120, 200)},  # rgb(0, 120, 200)

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
    if not 从哪行开始 <= id <= 到哪行结束:
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
            print('[ERROR] (line {0}) 未找到角色 "{1}"'.format(id, text[i][0]))
            return
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
            img.save("{0}-{1}/{2}.png".format("%d" % id, i, n))

    print("[Render] (line {0}) [{1}] Character: {2}".format(id, n, chara))
    if not break_every_sentence:
        img.save("{0}.png".format(id, "%d" % id, n))
    if AutoPreviewImage:
        img.show()


with open(input_file_name, encoding="utf-8") as f:
    reader = csv.reader(f, delimiter = ";")
    i = 0
    id = 0
    a = []
    b = []
    c = []
    while True:
        try:
            a.append(next(reader))
        except Exception:
            break
    for sent in a:
        length = int((len(sent) - 1) / 2)
        for i in range(0, length):
            b.append((sent[0], sent[i * 2 + 1], sent[i * 2 + 2]))
    c.append(b[0])
    for ind in range(0, len(b) - 1):
        if (
            i >= 3
            or (b[ind][0] == "Singing" and b[ind + 1][0] != "Singing")
            or (b[ind][0] != "Singing" and b[ind + 1][0] == "Singing")
        ):
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
