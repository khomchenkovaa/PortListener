CREATE TABLE DIAG_DATA (
       ID             BIGSERIAL PRIMARY KEY,
       CODE_KKS       VARCHAR(50) NOT NULL,
       PLACE          VARCHAR(50),
       ERROR_CODE     VARCHAR(50),
       REG_TIME       TIMESTAMPTZ NOT NULL DEFAULT NOW()
       );
