# First Signal User Provisioning

Initial First Signal users are created manually. There is no public registration, password reset, or account-administration UI in this pass.

## Create A User

1. In Firebase Authentication, create an Email/Password user and record the Firebase UID.
2. In Firestore, create `users/{uid}` with at least:

```json
{
  "email": "user@example.com",
  "displayName": "Display Name",
  "handle": "username",
  "handleNormalized": "username",
  "hasAccess": true,
  "roles": []
}
```

3. Create `loginHandles/{handleNormalized}` with:

```json
{
  "uid": "the-firebase-uid"
}
```

4. Keep `handleNormalized` lowercase. Allowed usernames are 3-32 characters and use letters, numbers, `.`, `_`, or `-`.
5. The headset's single identifier field accepts `username`, `@username`, or the Firebase email address with the same password.

## Safety

- Never put passwords, Firebase ID tokens, refresh tokens, or future join tickets in Unreal PlayerState or replicated properties.
- Set `hasAccess` deliberately. Roles and capabilities are server-owned; the login request cannot grant them.
- The deployed API may return `503` while Firebase billing is disabled. That is the expected current infrastructure state.
