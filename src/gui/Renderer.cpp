
void Renderer::setShuffle(bool shuffle)
{
    if (m_projectM) {
        m_projectM->setShuffleEnabled(shuffle);
    }
}
