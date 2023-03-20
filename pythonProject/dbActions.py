import sqlite3


def checkAwake(now, future):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            Select id 
            FROM resident
            where awake_time >= ? and awake_time < ? 
            """
        cursor = db_conn.cursor()
        arr = (now, future)
        cursor.execute(sql, arr)
        res = cursor.fetchall()
        list = []
        for x in res:
            list += x
        return list

def checkBedtime(now, future):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            Select id 
            FROM resident
            where bed_time >= ? and bed_time < ? 
            """
        cursor = db_conn.cursor()
        arr = (now, future)
        cursor.execute(sql, arr)
        res = cursor.fetchall()
        list = []
        for x in res:
            list += x
        return list

def logSensorData(topic, payload):
    parsed_data = parsePayload(topic, payload)
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            INSERT into sensor (resident_id, in_bed, duration)
            VALUES (?,?,?)
            """
        cursor = db_conn.cursor()
        cursor.execute(sql, parsed_data)
        db_conn.commit()


def logAlertData(id):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
            INSERT into alert (resident_id)
            VALUES (?)
            """
        cursor = db_conn.cursor()
        cursor.execute(sql, id)
        db_conn.commit()


def getResidentInfo(id):
    with sqlite3.connect("db.db") as db_conn:
        sql = """
                Select resident.id, name, room_nr, alert.id
                FROM resident inner join alert on resident.id = alert.resident_id
                where resident.id = ?
                order by alert.id desc 
                """
        cursor = db_conn.cursor()
        cursor.execute(sql, id)
        result = str(cursor.fetchone())\
            .replace("(", "") \
            .replace(")", "") \
            .replace("'", "")
        return result

def updateAlert(alertId, response):
    with sqlite3.connect("db.db") as db_conn:
        sql = """ 
                 Update alert 
                 set outcome = ?, resolved_at = datetime('now', 'localtime')
                 where id = ?
        """
        cursor = db_conn.cursor()
        cursor.execute(sql, [response, alertId])
        db_conn.commit()
def parsePayload(topic, payload):
    resident_id = topic.split("/")[1]
    splitPayload = payload.split(",")
    in_bed = splitPayload[1]
    duration = splitPayload[0]
    return resident_id, in_bed, duration