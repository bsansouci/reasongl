type buttonStateT =
  | LeftButton
  | MiddleButton
  | RightButton;

type stateT =
  | MouseDown
  | MouseUp;

type keycodeT =
  | Backspace
  | Tab
  | Enter
  | Escape
  | Space
  | Quote
  | Comma
  | Minus
  | Period
  | Slash
  | Num_0
  | Num_1
  | Num_2
  | Num_3
  | Num_4
  | Num_5
  | Num_6
  | Num_7
  | Num_8
  | Num_9
  | Semicolon
  | Equals
  | OpenBracket
  | Backslash
  | CloseBracket
  | A
  | B
  | C
  | D
  | E
  | F
  | G
  | H
  | I
  | J
  | K
  | L
  | M
  | N
  | O
  | P
  | Q
  | R
  | S
  | T
  | U
  | V
  | W
  | X
  | Y
  | Z
  | Right
  | Left
  | Down
  | Up
  | LeftCtrl
  | LeftShift
  | LeftAlt
  | LeftOsKey
  | RightCtrl
  | RightShift
  | RightAlt
  | RightOsKey
  | CapsLock
  | Backtick
  | Nothing;

let keycodeMap: int => keycodeT =
  fun
  | 8 => Backspace
  | 9 => Tab
  | 13 => Enter
  | 27 => Escape
  | 32 => Space
  | 39 => Quote
  | 44 => Comma
  | 45 => Minus
  | 46 => Period
  | 47 => Slash
  | 48 => Num_0
  | 49 => Num_1
  | 50 => Num_2
  | 51 => Num_3
  | 52 => Num_4
  | 53 => Num_5
  | 54 => Num_6
  | 55 => Num_7
  | 56 => Num_8
  | 57 => Num_9
  | 59 => Semicolon
  | 61 => Equals
  | 91 => OpenBracket
  | 92 => Backslash
  | 93 => CloseBracket
  | 96 => Backtick
  | 97 => A
  | 98 => B
  | 99 => C
  | 100 => D
  | 101 => E
  | 102 => F
  | 103 => G
  | 104 => H
  | 105 => I
  | 106 => J
  | 107 => K
  | 108 => L
  | 109 => M
  | 110 => N
  | 111 => O
  | 112 => P
  | 113 => Q
  | 114 => R
  | 115 => S
  | 116 => T
  | 117 => U
  | 118 => V
  | 119 => W
  | 120 => X
  | 121 => Y
  | 122 => Z
  | 1073741881 => CapsLock
  | 1073741903 => Right
  | 1073741904 => Left
  | 1073741905 => Down
  | 1073741906 => Up
  | 1073742048 => LeftCtrl
  | 1073742049 => LeftShift
  | 1073742050 => LeftAlt
  | 1073742051 => LeftOsKey
  | 1073742052 => RightCtrl
  | 1073742053 => RightShift
  | 1073742054 => RightAlt
  | 1073742055 => RightOsKey
  | _ => Nothing;
