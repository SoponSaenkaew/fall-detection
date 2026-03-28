package handler

import (
	"net/http"

	"backend/internal/model"
	"backend/internal/service"

	"github.com/gin-gonic/gin"
)

type EventHandler struct {
	service *service.EventService
}

func NewEventHandler(s *service.EventService) *EventHandler {
	return &EventHandler{
		service: s,
	}
}

func (h *EventHandler) HandleEvent(c *gin.Context) {
	var e model.Event

	if err := c.ShouldBindJSON(&e); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	result := h.service.ProcessEvent(e)

	c.JSON(http.StatusOK, gin.H{
		"status": result,
	})
}
