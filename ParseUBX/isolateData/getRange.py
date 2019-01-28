__author__ = 'Michael Fu'

import csv

infilename = "..//Home_20151220_0944.csv"
outfilename = infilename[:-4] + "(range).csv"

with open(infilename, 'rb') as infile:
       lines = infile.readlines()

filewriter = csv.writer(open(outfilename, 'wb'))

for l in lines:
    lineList = l.split(',')
    if lineList[0] == 'RXM' and lineList[1] == 'RAW':
        lineList[2] = int(round(int(lineList[2]), -3) / 1000)
        lineList[-1] = lineList[-1][:lineList[-1].rindex('\r')]

        timeList = lineList[2:5]
        filewriter.writerow(timeList)

        rangeList = lineList[5:]
        filewriter.writerow(rangeList)



