from datetime import datetime, timedelta

from dbSetup import checkAwake

start_time = datetime.utcnow()
now = datetime.utcnow()

time = datetime.now()
time5 = time + timedelta(minutes=30)
current_time = time.strftime("%H:%M:%S")
res = checkAwake(current_time, time5)

print("Start")
for x in res:
    print(x)
