fname = 'COM3_140326_180748'
f = open(fname+'.csv')
lines = f.readlines()
type2outf = {}
for line in lines:
	line = line.strip()
	ws = line.split(',')
	type = ws[0] + '_' + ws[1]
	if not type2outf.has_key(type):
		type2outf[type] = open(fname+type+'.csv','w')
	if line.count(',') < 2:
		continue
	line = line[line.index(',',line.index(',')+1)+1:]
	print>>type2outf[type],line
	