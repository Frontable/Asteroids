class ExperienceSystem
{
public:
    ExperienceSystem(Frost::Registry& registry, Frost::Entity playerEntity)
        : m_Registry(registry)
        , m_PlayerEntity(playerEntity) {
    }

    void Update(float dt) {}

    void AwardXP(AsteroidTag::Size size)
    {
        if (!m_Registry.IsAlive(m_PlayerEntity)) return;
        if (!m_Registry.Has<Experience>(m_PlayerEntity)) return;

        auto& xp = m_Registry.Get<Experience>(m_PlayerEntity);

        int amount = 0;
        switch (size)
        {
        case AsteroidTag::Size::Large: amount = Experience::XP_LARGE;  break;
        case AsteroidTag::Size::Medium: amount = Experience::XP_MEDIUM; break;
        case AsteroidTag::Size::Small: amount = Experience::XP_SMALL;  break;
        }

        xp.current += amount;

        while (xp.current >= xp.threshold)
        {
            xp.current -= xp.threshold;
            xp.level++;
            xp.threshold = (int)(xp.threshold * 1.5f); 

            FROST_LOG("Level up! Now level %d (next: %d XP)",
                xp.level, xp.threshold);

            Frost::EventBus::Emit(PlayerLevelUpEvent{
                xp.level,
                xp.threshold
                });
        }
    }

private:
    Frost::Registry& m_Registry;
    Frost::Entity m_PlayerEntity;
};