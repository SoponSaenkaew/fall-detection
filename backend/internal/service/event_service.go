package service

import (
	"backend/internal/model"
	"backend/pkg/line"
	"backend/pkg/sheets"
)

type EventService struct {
	roomState  map[string]bool
	lineClient *line.Client
	sheets     *sheets.Client
	targetUser string
}

func NewEventService(l *line.Client, s *sheets.Client, userID string) *EventService {
	return &EventService{
		roomState:  make(map[string]bool),
		lineClient: l,
		sheets:     s,
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
		go s.sheets.Send(e.DeviceID, "enter", e.Timestamp)
		return "enter"

	case "exit":
		s.roomState[e.DeviceID] = false
		go s.sheets.Send(e.DeviceID, "exit", e.Timestamp)
		return "exit"

	case "fall":
		if s.roomState[e.DeviceID] {

			if s.lineClient != nil {
				_ = s.lineClient.Push(
					s.targetUser,
					"⚠️ มีคนล้มใน "+e.DeviceID,
				)
			}

			go s.sheets.Send(e.DeviceID, "fall", e.Timestamp)
			return "real_fall"
		}
		return "ignore_fall"

	default:
		return "unknown"
	}
}
