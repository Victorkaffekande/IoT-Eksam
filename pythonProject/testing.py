from datetime import datetime
from dbSetup import *

start_time = datetime.now()
now = datetime.now()

print(start_time.minute)
print(now.minute)



createTables()
residents = [("Jimmy", 202, "07:00:00", "10:00:00"), ("Clara", 105, "07:00:00", "10:05:00"),
             ("John", 109, "13:20:00", "09:15:00")]
setupPeople(residents)
