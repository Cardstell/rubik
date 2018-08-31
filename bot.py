#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import requests, vk, time, kociemba, subprocess, math, shutil, colorsys
import numpy as np, cv2 as cv

debug = 1
width = 1600
height = 900
red = 0.01
orange = 0.05
commands = ['calib']
api = None
ddirect = "URFDLB"
dcolors = "YRBWOG"
starttime = 0
token = ""

def solve(cube, format):
    sol = ""
    try:
        sol = kociemba.solve(cube)
    except:
        print("fail")
        return "Не удалось распознать куб"
    with open("cache/shared", 'w') as f:
        f.write(cube)
    print("success in %.2fs" % (time.time() - starttime))
    if (format == 0): return sol

def angle_cos(p0, p1, p2):
    d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
    return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

def find_squares(img):
    #img = cv.GaussianBlur(img, (3, 3), 0)
    squares = []
    for gray in cv.split(img):
        for thrs in range(0, 255, 8):
            if thrs == 0:
                bin = cv.Canny(gray, 0, 50, apertureSize=5)
                bin = cv.dilate(bin, None)
            else:
                _retval, bin = cv.threshold(gray, thrs, 255, cv.THRESH_BINARY)
            if (debug == 2):
                cv.imshow("bin",bin)
                cv.waitKey()
            bin, contours, _hierarchy = cv.findContours(bin, cv.RETR_LIST, cv.CHAIN_APPROX_SIMPLE)
            for cnt in contours:
                cnt_len = cv.arcLength(cnt, True)
                cnt = cv.approxPolyDP(cnt, 0.02*cnt_len, True)
                if len(cnt)==4 and cv.contourArea(cnt) > 10000 and cv.isContourConvex(cnt):
                    cnt = cnt.reshape(-1, 2)
                    max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in range(4)])
                    if max_cos < 0.3:
                        squares.append(cnt)
    return squares

def insidesq(sq, points):
    for pt in points:
        for i in range(2):
            a = (sq[0][0] - pt[0]) * (sq[1][1] - sq[0][1]) - (sq[1][0] - sq[0][0]) * (sq[0][1] - pt[1])
            b = (sq[1][0] - pt[0]) * (sq[2][1] - sq[1][1]) - (sq[2][0] - sq[1][0]) * (sq[1][1] - pt[1])
            c = (sq[2][0] - pt[0]) * (sq[0][1] - sq[2][1]) - (sq[0][0] - sq[2][0]) * (sq[2][1] - pt[1])
            if ((a >= 0 and b >= 0 and c >= 0) or (a <= 0 and b <= 0 and c <= 0)): return True
            tmp = sq[3]
            sq[3] = sq[1]
            sq[1] = tmp
    return False

def inside(sqs, pt):
    for sqp in sqs:
        sq = list(sqp)
        for i in range(2):
            a = (sq[0][0] - pt[0]) * (sq[1][1] - sq[0][1]) - (sq[1][0] - sq[0][0]) * (sq[0][1] - pt[1])
            b = (sq[1][0] - pt[0]) * (sq[2][1] - sq[1][1]) - (sq[2][0] - sq[1][0]) * (sq[1][1] - pt[1])
            c = (sq[2][0] - pt[0]) * (sq[0][1] - sq[2][1]) - (sq[0][0] - sq[2][0]) * (sq[2][1] - pt[1])
            if ((a >= 0 and b >= 0 and c >= 0) or (a <= 0 and b <= 0 and c <= 0)): return True
            tmp = sq[3]
            sq[3] = sq[1]
            sq[1] = tmp
    return False

def avg(pt1, pt2):
    return [(pt1[0]+pt2[0])//2, (pt1[1]+pt2[1])//2]

def recognize(images, calib=False):
    result = ["000000000"]*6
    success = [0]*6
    global red, orange
    for cnt in range(len(images)):
        image = images[cnt]
        subprocess.check_output(["ffmpeg","-i","cache/"+image,"-vf","scale="+str(width)+":"+str(height),"cache/image.png","-y"],stderr=subprocess.STDOUT)
        img = cv.imread("cache/image.png",0)
        squares = find_squares(img)
        newsquares = []
        points = []
        for sq in squares:
            minsize = 10**10
            maxsize = 0
            point = [0, 0]
            for j in range(4):
                cursize = math.sqrt((sq[j][0]-sq[j-1][0])**2 + (sq[j][1] - sq[j-1][1])**2)
                maxsize = max(maxsize, cursize)
                minsize = min(minsize, cursize)
                point[0] += sq[j][0]
                point[1] += sq[j][1]
            point[0] //= 4
            point[1] //= 4
            if (minsize<0.05*max(width, height)): continue
            if (maxsize>0.5*min(width, height)): continue
            if (point[0]<0.15*width or point[0]>0.85*width): continue
            if (insidesq(list(sq), points)): continue
            if (inside(list(newsquares), point)): continue
            newsquares.append(sq)
            points.append(point)
        if debug:
            cv.drawContours( img, newsquares, -1, (255, 255, 0), 3 )
            cv.imshow('squares', img)
            cv.waitKey()
            cv.destroyAllWindows()
        if (len(points) != 9): continue
        gr = ["0"]*9
        skeys = sorted(points, key=lambda x: x[0])
        skeys = [sorted(skeys[i*3:(i+1)*3], key=lambda x: x[1]) for i in range(3)]
        indices = [0,3,6,1,4,7,2,5,8]
        img = cv.imread("cache/image.png",1)
        img = cv.GaussianBlur(img, (21, 21), 0)
        newimg = blank_image = np.zeros((900,900,3), np.uint8)
        for i in range(9):
            cursq = []
            center = list(skeys[i//3][i%3])
            for j in range(9):
                if (points[j] == center):
                    cursq = list(newsquares[j])
                    break
            for j in range(4):
                cursq[j] = avg(cursq[j], center)
            avgr = 0
            avgg = 0
            avgb = 0
            cursqx = sorted(cursq, key = lambda x: x[0])
            cursqy = sorted(cursq, key = lambda x: x[1])
            pixels = 0
            for y in range(cursqy[0][1], cursqy[3][1]):
                for x in range(cursqx[0][0], cursqx[3][0]):
                    pxl = img[y,x]
                    r = int(pxl[2])
                    g = int(pxl[1])
                    b = int(pxl[0])
                    avgr += r
                    avgg += g
                    avgb += b
                    pixels += 1
            #colorsys.rgb_to_hls()
            avgr //= pixels
            avgg //= pixels
            avgb //= pixels
            curh,curl,curs = colorsys.rgb_to_hls(avgr/255,avgg/255,avgb/255)
            #print(curh, curl, curs)
            newr, newg, newb = colorsys.hls_to_rgb(curh, 0.5, 1.0)
            if curs<0.3 and curl>0.5:
                newr = 255
                newg = 255
                newb = 255
            cv.rectangle(newimg, ((i//3)*300, (i%3)*300), ((i//3+1)*300-1, (i%3+1)*300-1), (newb*255,newg*255,newr*255), -1)
            global red, orange
            if (calib and i==0): red = curh
            if (calib and i==8): orange = curh
            curcolor = '0'

            if curs<0.3 and curl>0.5: curcolor = 'W'
            elif curh<0.1:
                if (curl<0.35): curcolor = 'R'
                else: curcolor = 'O'
            elif abs(curh-0.15)<0.04: curcolor = 'Y'
            elif curh>0.25 and curh<0.4: curcolor = 'G'
            elif curh>0.4 and curh<0.6: curcolor = 'B'
            gr[indices[i]] = curcolor
        gr = "".join(gr)
        cv.imshow("",newimg)
        cv.waitKey()
        cv.destroyAllWindows()
        print(str(cnt+1)+": "+gr)
        if ("0" not in gr):
            ind = dcolors.index(gr[4])
            result[ind] = gr
            success[ind] = 1
    if calib:
        if (sum(success) != 0):
            print(("calibrate: rh=%.3f "%red)+("oh=%.3f"%orange))
        else: print("calib fail")
        return ""
    if sum(success)!= 6:
        print("fail")
        return "fail"
    res = ""
    for i in "".join(result):
        res += ddirect[dcolors.index(i)]
    return res

def tcb(callback, *args, **kwargs):
    for i in range(10):
        try:
            return callback(*args, **kwargs)
        except:
            time.sleep(4)

def main():
    counter = 0
    api = vk.API(vk.Session(access_token=token))
    while True:
        if (counter%15 == 0):
            flg = False
            with open("cache/sigexit", 'r') as f:
                flg = (f.read()[0] == "1")
            if (flg):
                with open("cache/sigexit", 'w') as f:
                    f.write("0")
                return
        messages = tcb(api.messages.get,count=30)
        del messages[0]
        for ms in messages:
            if (ms['out']==0 and ms['read_state']==0):
                comm = ""
                if (ms['body'] != ""): comm = ms['body'].lower().split()[0]
                if (comm in commands):
                    if (comm == 'calib' and ms.get('attachments', -1) != -1):
                        if (ms['attachments'][0]['type'] != 'photo'): continue
                        r = tcb(requests.get, ms['attachments'][0]['photo']['src_xxxbig'], stream=True)
                        if (r != None):
                            if r.status_code == 200:
                                with open("cache/image0.jpg", 'wb') as f:
                                    r.raw.decode_content = True
                                    shutil.copyfileobj(r.raw, f)
                        recognize(["image0.jpg"], calib=True)
                        tcb(api.messages.markAsRead,peer_id=ms['uid'])
                elif (ms.get('attachments', -1) != -1):
                    if (len(ms['attachments'])<6 and not debug):
                        tcb(api.messages.send, user_id=ms['uid'], message='Недостаточно изображений для распознания куба')
                        continue
                    global starttime
                    starttime = time.time()
                    photos = []
                    cnt = 0
                    for attach in ms['attachments']:
                        if (attach['type'] != 'photo'): continue
                        for ind in ('src_xxxbig', 'src_xxbig', 'src_xbig', 'src_big', 'src_small'):
                            if (attach['photo'].get(ind, -1) == -1): continue
                            while (True):
                                r = tcb(requests.get, attach['photo'][ind], stream=True)
                                if (r != None):
                                    if r.status_code == 200:
                                        with open("cache/image"+str(cnt)+".jpg", 'wb') as f:
                                            r.raw.decode_content = True
                                            shutil.copyfileobj(r.raw, f)
                                        break
                                time.sleep(4)
                            photos.append("image"+str(cnt)+".jpg")
                            cnt += 1
                            break
                    if (len(photos)<6 and not debug):
                        tcb(api.messages.send, user_id=ms['uid'], message='Недостаточно изображений для распознания куба')
                        continue
                    #tcb(api.messages.markAsRead,peer_id=ms['uid'])
                    result = recognize(photos)
                    if (result == "fail"):
                        tcb(api.messages.send, user_id=ms['uid'], message='Не удалось распознать куб')
                    else:
                        msg = solve(result, 0)
                        tcb(api.messages.send, user_id=ms['uid'], message=msg)
                else:
                    tcb(api.messages.markAsRead,peer_id=ms['uid'])
        time.sleep(1.5)
        counter += 1

if __name__ == "__main__":
    main()
