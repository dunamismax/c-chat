# C-Chat Network Protocol Specification

## Overview

C-Chat uses a custom TCP-based protocol for secure communication between clients and server. The server acts as a relay for encrypted messages while never having access to plaintext content.

## Message Format

All network messages use the following binary format:

```
[4 bytes: Message Length][1 byte: Message Type][N bytes: Payload]
```

- **Message Length**: 32-bit big-endian integer indicating total payload size
- **Message Type**: 8-bit identifier for message type
- **Payload**: Variable-length data specific to message type

## Message Types

### Client to Server Messages

#### 0x01 - REGISTER_USER

Register a new user with the server.

```
Payload:
[1 byte: Username Length][N bytes: Username][32 bytes: Public Key]
```

#### 0x02 - LOGIN_USER

Authenticate with the server using cryptographic challenge.

```
Payload:
[1 byte: Username Length][N bytes: Username][64 bytes: Signature]
```

#### 0x03 - GET_PUBLIC_KEY

Request another user's public key.

```
Payload:
[1 byte: Username Length][N bytes: Username]
```

#### 0x04 - SEND_MESSAGE

Send encrypted message to another user.

```
Payload:
[1 byte: Recipient Length][N bytes: Recipient][2 bytes: Message Length][N bytes: Encrypted Message]
```

#### 0x05 - GET_MESSAGES

Poll for pending messages.

```
Payload: Empty
```

#### 0x06 - SET_STATUS

Update user presence status.

```
Payload:
[1 byte: Status] (0=offline, 1=online, 2=away)
```

#### 0x07 - LIST_USERS

Request list of registered users and their status.

```
Payload: Empty
```

#### 0x08 - LOGOUT

Disconnect from server gracefully.

```
Payload: Empty
```

### Server to Client Messages

#### 0x81 - REGISTER_RESPONSE

Response to user registration.

```
Payload:
[1 byte: Success] (0=failure, 1=success)
[1 byte: Error Code] (if failure)
```

#### 0x82 - LOGIN_RESPONSE

Response to login request with challenge.

```
Payload:
[1 byte: Success] (0=failure, 1=success)
[32 bytes: Challenge] (if success)
```

#### 0x83 - PUBLIC_KEY_RESPONSE

Response with requested public key.

```
Payload:
[1 byte: Found] (0=not found, 1=found)
[32 bytes: Public Key] (if found)
```

#### 0x84 - MESSAGE_ACK

Acknowledgment of message delivery.

```
Payload:
[4 bytes: Message ID]
[1 byte: Status] (0=failed, 1=delivered, 2=queued)
```

#### 0x85 - INCOMING_MESSAGE

Deliver message to client.

```
Payload:
[4 bytes: Message ID]
[1 byte: Sender Length][N bytes: Sender]
[4 bytes: Timestamp]
[2 bytes: Message Length][N bytes: Encrypted Message]
```

#### 0x86 - USER_LIST_RESPONSE

List of users and their status.

```
Payload:
[2 bytes: User Count]
For each user:
  [1 byte: Username Length][N bytes: Username][1 byte: Status]
```

#### 0x87 - STATUS_UPDATE

Notification of user status change.

```
Payload:
[1 byte: Username Length][N bytes: Username][1 byte: New Status]
```

#### 0x88 - ERROR

General error message.

```
Payload:
[1 byte: Error Code]
[2 bytes: Error Message Length][N bytes: Error Message]
```

## Error Codes

- 0x01: Invalid username
- 0x02: User already exists
- 0x03: User not found
- 0x04: Authentication failed
- 0x05: Invalid message format
- 0x06: Rate limit exceeded
- 0x07: Server error
- 0x08: Connection terminated

## Security Considerations

- All message content is end-to-end encrypted using libsodium
- Server only handles encrypted payloads and metadata
- User authentication uses cryptographic signatures
- No plaintext passwords are transmitted
- Rate limiting prevents abuse

## Connection Flow

1. Client connects to server via TCP
2. Client sends REGISTER_USER or LOGIN_USER
3. Server responds with challenge for authentication
4. Client proves identity with signature
5. Client can now send/receive messages
6. Server relays encrypted messages between clients
7. Client sends LOGOUT before disconnecting
