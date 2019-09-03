import os, sys, time, timeit, json

# "C:/canada.json"
def ReadJsonFrom(strfile = "citm_catalog.json"):
	file = open(strfile, "r") 
	strResult = file.read() 
	print "string json len:", len(strResult)
	return strResult

def LoadAndSee():
	strJson = ReadJsonFrom("citm_catalog.json")
	strJson2 = ReadJsonFrom("canada.json")
	a_ = json.dumps(json.loads(strJson))
	b_ = json.dumps(json.loads(strJson2))
	elapsed_time, TrueCnt, FailedCnt, Cnt = 0, 0, 0, 100
	for i in range(Cnt):
		start_time = time.time()
		a = json.dumps(json.loads(strJson))
		b = json.dumps(json.loads(strJson2))
		if a == a_ and b == b_:
			TrueCnt += 1
		else:
			FailedCnt += 1
		#print i, ", ", 
		elapsed_time += time.time() - start_time

	print "elapsed_time: ", elapsed_time/Cnt, "TrueCnt:", TrueCnt, "FailedCnt:", FailedCnt 
	# elapsed_time:  0.221400001049 TrueCnt: 100 FailedCnt: 0
	strInput = raw_input("See you ram. Nhap enter to next.")
	
if __name__ == "__main__":
	LoadAndSee()
