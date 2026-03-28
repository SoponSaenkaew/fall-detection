package main

import (
	"backend/internal/handler"
	"backend/internal/service"
	"backend/pkg/line"

	"backend/pkg/sheets"

	"github.com/gin-gonic/gin"
)

func main() {

	sheetsClient := sheets.NewClient(
		"https://script.google.com/macros/s/AKfycbyY3la2qKDCTdnLBcuUV5nJNCnquWZdetW0C-vZu-mAS7DE4rZxSwWVq1489b3lcM3r/exec",
	)
	// 🔥 สร้าง LINE client
	lineClient, err := line.NewClient(
		"d129268d4cedc97dfcfd66cd1a16100e",
		"z7vLFbNbBCgXfNpIXA8CHXYxV6yEA0pDppgvd5cZlsU43lIox5JrtXe+uMRhK9kuAZmZ/J9/RYT/dZZAM92URbNC5Ji5XFWSNamVyYjHS2ZdfoiSlXoEQ4n1oCAV6W8LDM1Y1cZTATleos+X0/3DewdB04t89/1O/w1cDnyilFU=",
	)
	if err != nil {
		panic(err)
	}

	// 🔥 inject เข้า service
	eventService := service.NewEventService(
		lineClient,
		sheetsClient,
		"Ue6febb4090fb19862873194265adbfe2",
	)

	// 🔥 inject เข้า handler
	eventHandler := handler.NewEventHandler(eventService)

	r := gin.Default()
	r.POST("/event", eventHandler.HandleEvent)

	r.Run(":8080")
}
