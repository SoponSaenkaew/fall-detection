package ws

import (
	"net/http"
	"sync"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true }, // ยอมรับการเชื่อมต่อจาก Next.js
}

var clients = make(map[*websocket.Conn]bool)
var broadcast = make(chan interface{})
var mutex sync.Mutex

// ฟังก์ชันให้หน้าเว็บเชื่อมต่อ
func HandleConnections(c *gin.Context) {
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		return
	}
	defer ws.Close()

	mutex.Lock()
	clients[ws] = true
	mutex.Unlock()

	for {
		if _, _, err := ws.ReadMessage(); err != nil {
			mutex.Lock()
			delete(clients, ws)
			mutex.Unlock()
			break
		}
	}
}

// ฟังก์ชันคอยรอกระจายข้อมูลไปทุกหน้าเว็บ
func HandleMessages() {
	for {
		msg := <-broadcast
		mutex.Lock()
		for client := range clients {
			if err := client.WriteJSON(msg); err != nil {
				client.Close()
				delete(clients, client)
			}
		}
		mutex.Unlock()
	}
}

// ฟังก์ชันสำหรับโยนข้อมูลเข้ามาในระบบกระจายข้อความ
func BroadcastEvent(data interface{}) {
	broadcast <- data
}
