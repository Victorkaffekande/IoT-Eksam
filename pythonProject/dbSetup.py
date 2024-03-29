import sqlite3

def createTables():
    db_conn = sqlite3.connect("db.db")
    sql = """
        CREATE TABLE resident (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            room_nr INTEGER NOT NULL,
            awake_time Text default "07:00:00",
            bed_time Text default "10:00:00"
        )
        """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()

    sql = """
                CREATE TABLE sensor (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                resident_id INTEGER NOT NULL,
                created_at DATE default (datetime('now', 'localtime')) NOT NULL,
                in_bed bool NOT NULL,
                duration int not null, 
                FOREIGN KEY (resident_id) REFERENCES resident (id)
                )
                """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()

    sql = """
                    CREATE TABLE alert (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    resident_id INTEGER NOT NULL,
                    created_at DATE default (datetime('now', 'localtime')) NOT NULL,
                    resolved_at DATE,
                    outcome bool,
                    FOREIGN KEY (resident_id) REFERENCES resident(id)
                    )
                    """
    cursor = db_conn.cursor()
    cursor.execute(sql)
    cursor.close()
    db_conn.close()


def setupPeople(residents):
    db_conn = sqlite3.connect("db.db")
    sql = """
    INSERT INTO resident(name, room_nr,awake_time,bed_time) 
    values (?,?,?,?)
    """
    cursor = db_conn.cursor()
    cursor.executemany(sql, residents)
    db_conn.commit()
    db_conn.close()


residents = [("Jimmy", 202, "07:00:00", "10:00:00"), ("Clara", 105, "07:00:00", "10:05:00"),
             ("John", 109, "13:20:00", "09:15:00")]
setupPeople(residents)

