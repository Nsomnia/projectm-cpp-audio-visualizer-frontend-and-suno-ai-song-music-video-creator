
void MainWindow::nextPreset()
{
    if (m_renderer) {
        m_renderer->selectNextPreset();
    }
}

void MainWindow::prevPreset()
{
    if (m_renderer) {
        m_renderer->selectPreviousPreset();
    }
}
