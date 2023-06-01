CREATE TABLE COUNTERS (
       ID             BIGSERIAL PRIMARY KEY,
       CODE_KKS       VARCHAR(50) NOT NULL,
       SIGNAL_CURRENT REAL NOT NULL DEFAULT 0,
       TEMPERATURE    REAL NOT NULL DEFAULT 0,
       REG_TIME       TIMESTAMPTZ NOT NULL DEFAULT NOW()
       );