# Passwds
### A secure password manager.

---
![Passwds](https://github.com/ThatLukeDev/passwds/assets/76230394/2a86e115-67a0-4441-bf84-bd7c760ef6f4)

---

- To use this app:
  - Clone the [repo](https://github.com/ThatLukeDev/passwds.git) with `git clone https://github.com/ThatLukeDev/passwds.git`.
  - Compile the source code in the base directory with `./run`.
  - Run the compiled `passwds.out` executable file.

---

Once opened, the startup screen will ask you for a master password.
  - If this is the first time opening, it will prompt you to re-enter your chosen password.
  - All data is encrypted by this master password and stored in `.passwds-passkey-store-tmp`.

---

- `Name` field.
  - This is the name your password is stored under.
- `Password` field.
  - This is the password stored under the name field.
- `Generate` button.
  - This generates a random password of any length (default is 24 characters).
- `Check` button.
  - This gives you a score of how secure your password is (100% is bare minimum).
- `Select` button.
  - This opens the list of stored passwords and allows you to load one.
- `+` button.
  - This adds a record with name from `Name` and password from `Password`.
- `-` button.
  - This removes the record with name as `Name`.

---

For any issues or feature requests, raise them in the *[github issues](https://github.com/ThatLukeDev/passwds/issues)* tab.
