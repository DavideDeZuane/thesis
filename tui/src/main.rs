use std::io::{self, stdout};

use crossterm::{
    event::{self, Event, KeyCode},
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
    ExecutableCommand,
};
use ratatui::{prelude::*, widgets::*};

fn main() -> io::Result<()> {
    enable_raw_mode()?;
    stdout().execute(EnterAlternateScreen)?;
    let mut terminal = Terminal::new(CrosstermBackend::new(stdout()))?;

    let mut should_quit = false;
    while !should_quit {
        terminal.draw(ui)?;
        should_quit = handle_events()?;
    }

    disable_raw_mode()?;
    stdout().execute(LeaveAlternateScreen)?;
    Ok(())
}

fn handle_events() -> io::Result<bool> {
    if event::poll(std::time::Duration::from_millis(50))? {
        if let Event::Key(key) = event::read()? {
            if key.kind == event::KeyEventKind::Press && key.code == KeyCode::Char('q') {
                return Ok(true);
            }
        }
    }
    Ok(false)
}

fn ui(frame: &mut Frame) {

    let vertical_layout = Layout::new(Direction::Horizontal, [
            Constraint::Percentage(30),
            Constraint::Percentage(70),
        ],).split(frame.size());

    let left_layout = Layout::new(Direction::Vertical,
        [Constraint::Percentage(50), Constraint::Percentage(50)],
    ).split(vertical_layout[0]);

    let rigth_layout = Layout::new(Direction::Vertical,
        [Constraint::Percentage(100)],
    ).split(vertical_layout[1]);

    frame.render_widget(
        Block::default().borders(Borders::ALL).title("Exchange"),
        rigth_layout[0],
    );
    frame.render_widget(
        Block::default().borders(Borders::ALL).title("Logs"),
        left_layout[0],
    );
    frame.render_widget(
        Block::default().borders(Borders::ALL).title("Running"),
        left_layout[1],
    );
}
