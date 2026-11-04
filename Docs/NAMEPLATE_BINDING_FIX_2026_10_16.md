# Nameplate Placeholder Repair

## Finding

Read-only inspection of AFCore `Widget_NameTag` confirms that
`Updated_PlayerName_Event` has no connected implementation. The bound
`Updated_PlayerName_Event_0` calls SetText on `AFCore_Text_C_25`.
WMCYN was invoking the empty event, leaving the designer's `Text Block` default.
This is a binding defect, not evidence of a font glyph problem.

## Correction

WMCYN's presence adapter invokes the implemented event. Reflected text/string
parameters now use FStructOnScope for initialization and destruction.
AFCore assets, authentication, pawn tracking, and name selection are unchanged.

## Acceptance

- Log in and inspect the mirror: nameplate must show the resolved PlayerState
  display name, not `Text Block`.
- Confirm expected username versus profile display name with the account data;
  do not hardcode a name into the widget.
- Test another user's replicated label with matching client/server builds.

The latest editor test also logged NetChecksumMismatch on
WMCYN_FirstSignalPresence against the deployed server. Remote acceptance is
blocked until matching builds are deployed; do not disable checksum checks.
