package line

import "github.com/line/line-bot-sdk-go/v7/linebot"

type Client struct {
	bot *linebot.Client
}

func NewClient(secret, token string) (*Client, error) {
	bot, err := linebot.New(secret, token)
	if err != nil {
		return nil, err
	}
	return &Client{bot: bot}, nil
}

func (c *Client) Push(userID string, msg string) error {
	_, err := c.bot.PushMessage(
		userID,
		linebot.NewTextMessage(msg),
	).Do()

	return err
}
