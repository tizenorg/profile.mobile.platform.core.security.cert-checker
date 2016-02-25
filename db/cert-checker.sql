BEGIN EXCLUSIVE TRANSACTION;

-- Update here on every schema change! Integer value.
PRAGMA user_version = 1;

-- Table 'to_check'
CREATE TABLE IF NOT EXISTS to_check (
  check_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  app_id   TEXT NOT NULL,
  pkg_id   TEXT NOT NULL,
  uid      INTEGER NOT NULL,
  verified INTEGER NOT NULL,

  UNIQUE (app_id, pkg_id, uid) ON CONFLICT REPLACE
);

-- Table 'chains_to_check'
CREATE TABLE IF NOT EXISTS chains_to_check (
  chain_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  check_id INTEGER NOT NULL,

  FOREIGN KEY (check_id) REFERENCES to_check(check_id) ON DELETE CASCADE
);

-- Table 'certs_to_check'
CREATE TABLE IF NOT EXISTS certs_to_check (
  chain_id    INTEGER NOT NULL,
  certificate TEXT NOT NULL,
  cert_order  INTEGER NOT NULL,

  UNIQUE (chain_id, cert_order),

  PRIMARY KEY (chain_id, certificate),
  FOREIGN KEY (chain_id) REFERENCES chains_to_check(chain_id) ON DELETE CASCADE
);

COMMIT TRANSACTION;
