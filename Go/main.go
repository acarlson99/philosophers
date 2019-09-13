package main

import (
	"fmt"
	"sync"
	"time"
)

type fork struct {
	sync.Mutex
	state state
}

/* Constants for HP, and times */
var (
	MAXHP                 = 10
	TIMEOUT time.Duration = 10
	EATING  time.Duration = 2
	RESTING time.Duration = 2
)

type state int

const (
	none state = iota
	sleep
	eating
	rest
	thinking
	waiting
	clean
	dirty
)

type philosopher struct {
	id, hp      int
	right, left *fork
	state       state
	sem         chan available
}

func (p *philosopher) eat() {
	p.right.Lock()
	p.left.Lock()

	p.state = eating
	p.hp = MAXHP
	time.Sleep(EATING * time.Second)
	p.right.Unlock()
	p.left.Unlock()
}

func (p *philosopher) rest() {
	p.state = rest
	time.Sleep(RESTING * time.Second)
}

func (p *philosopher) dine() {
	for {
		p.state = waiting
		availableSpot := <-p.sem
		p.eat()
		p.sem <- availableSpot
		p.rest()
	}
}

// increment
// shelf <- gymbag
// do something
// <- shelf
// decrement
// cap := <- caps
// do something
// caps <- cap

func overSeer(philosophers []*philosopher, dead chan int) {
	for {
		for i, x := range philosophers {
			if x.state != eating {
				x.hp--
			}
			if x.hp <= 0 {
				dead <- i
			}
		}
		time.Sleep(time.Second)
	}
}

type available struct{}

func main() {
	// defer profile.Start(profile. TraceProfile, profile.ProfilePath(".")).Stop()
	num := 7
	fork0 := &fork{state: dirty}
	left := fork0
	philosophers := make([]*philosopher, num)
	sem := make(chan available, num-1)
	for x := 0; x < (num - 1); x++ {
		sem <- available{}
	}
	for x := 1; x < num; x++ {
		right := &fork{state: dirty}
		phil := &philosopher{id: x, hp: MAXHP, right: right, left: left, state: none, sem: sem}
		philosophers[x] = phil
		go phil.dine()
		left = right
	}
	phil := &philosopher{id: 0, hp: MAXHP, right: fork0, left: left, state: none, sem: sem}
	philosophers[0] = phil
	dead := make(chan int, 1)
	done := make(chan string, 1)

	go phil.dine()
	go overSeer(philosophers, dead)
	var wg sync.WaitGroup
	wg.Add(1)
	go vis(philosophers, done, &wg)

	select {
	case who := <-dead:
		done <- fmt.Sprintf("index %d died\n", who)
	case <-time.After(TIMEOUT * time.Second):
		done <- fmt.Sprintf("Time to dance, no one died... Yay\n")
	}
	wg.Wait()
}
