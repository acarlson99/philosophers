package main

import (
	"log"
	"sync"
	"time"

	ui "github.com/gizak/termui/v3"
	"github.com/gizak/termui/v3/widgets"
)

func vis(philosophers []*philosopher, done chan string, wg *sync.WaitGroup) {
	if err := ui.Init(); err != nil {
		log.Fatalf("failed to init termui: %v", err)
	}
	defer ui.Close()
	defer wg.Done()

	gs := make([]*widgets.Gauge, 7)
	for i := range gs {
		gs[i] = widgets.NewGauge()
		gs[i].Percent = 100
		gs[i].BarColor = ui.ColorRed //based on sate?
	}
	p := widgets.NewParagraph()
	p.Text = "OUTPUT"
	grid := ui.NewGrid()
	termWidth, termHeight := ui.TerminalDimensions()
	grid.SetRect(0, 0, termWidth, termHeight)
	grid.Set(
		ui.NewCol(1.0/2,
			ui.NewRow(.9/7, gs[0]),
			ui.NewRow(.9/7, gs[1]),
			ui.NewRow(.9/7, gs[2]),
			ui.NewRow(.9/7, gs[3]),
			ui.NewRow(.9/7, gs[4]),
			ui.NewRow(.9/7, gs[5]),
			ui.NewRow(.9/7, gs[6])),
		ui.NewCol(1.0/2,
			ui.NewRow(.9/7, p)),
	)

	ui.Render(grid)
	uiEvents := ui.PollEvents()
	ticker := time.NewTicker(time.Second).C
	for {
		select {
		case e := <-uiEvents:
			switch e.ID {
			case "<Resize>":
				payload := e.Payload.(ui.Resize)
				grid.SetRect(0, 0, payload.Width, payload.Height)
				ui.Clear()
				ui.Render(grid)
			}
		case tmp := <-done:
			updateBars(philosophers, gs)
			p.Text = tmp
			ui.Render(grid)
			ui.Clear()
			for {
				q := <-uiEvents
				switch q.ID {
				case "q":
					return
				}
			}
		case <-ticker:
			updateBars(philosophers, gs)
			ui.Render(grid)
		}
	}

}

func updateBars(philosophers []*philosopher, gs []*widgets.Gauge) {
	var percent float32
	for i, x := range philosophers {
		percent = float32(x.hp) / float32(MAXHP) * 100
		gs[i].Percent = int(percent)
		if x.state == eating {
			gs[i].BarColor = ui.ColorGreen
			gs[i].Title = "EATING"
		} else if x.state == rest {
			gs[i].Title = "RESTING"
			gs[i].BarColor = ui.ColorRed
		} else if x.state == waiting {
			gs[i].Title = "WAITING TO BE SEETED"
			gs[i].BarColor = ui.ColorYellow
		} else {
			gs[i].BarColor = ui.ColorBlue
		}
	}
}
