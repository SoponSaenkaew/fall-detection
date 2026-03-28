package service

import (
	"backend/internal/model"
	"backend/pkg/line"
)

type EventService struct {
	roomState  map[string]bool
	lineClient *line.Client
	targetUser string
}

func NewEventService(l *line.Client, userID string) *EventService {
	return &EventService{
		roomState:  make(map[string]bool),
		lineClient: l,
		targetUser: userID,
	}
}

func (s *EventService) ProcessEvent(e model.Event) string {

	if _, ok := s.roomState[e.DeviceID]; !ok {
		s.roomState[e.DeviceID] = false
	}

	switch e.EventType {

	case "enter":
		s.roomState[e.DeviceID] = true
		return "enter"

	case "exit":
		s.roomState[e.DeviceID] = false
		return "exit"

	case "fall":
		if s.roomState[e.DeviceID] {

			if s.lineClient != nil {
				_ = s.lineClient.Push(
					s.targetUser,
					"⚠️ มีคนล้มใน "+e.DeviceID,
				)
			}

			return "real_fall"
		}
		return "ignore_fall"

	default:
		return "unknown"
	}
}
