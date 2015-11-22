% This LilyPond file was generated by Rosegarden 15.12
\include "nederlands.ly"
\version "2.12.0"
\header {
    composer = "D. Michael McIntyre"
    copyright =  \markup { "Copyright "\char ##x00A9" 2006, 2008 D. Michael McIntyre" }
    subtitle = "demonstrating multiple voices in both hands, and alternate endings"
    title = "Lilypond Export Demo #1"
    tagline = "Created using Rosegarden 15.12 and LilyPond"
}
#(set-global-staff-size 18)
#(set-default-paper-size "a4")
global = { 
    \time 4/4
    
}
globalTempo = {
    \override Score.MetronomeMark #'transparent = ##t
    \tempo 4 = 130  
}
\score {
    << % common
        \context GrandStaff = "0" << 
            % Force offset of colliding notes in chords:
            \override Score.NoteColumn #'force-hshift = #1.0
            % Allow fingerings inside the staff (configured from export options):
            \override Score.Fingering #'staff-padding = #'()

            \context Staff = "track 1, Piano" << 
                \set Staff.instrumentName = \markup { \center-column { "Piano " } }
                \set Staff.midiInstrument = "Acoustic Grand Piano"
                \set Score.skipBars = ##t
                \set Staff.printKeyCancellation = ##f
                \new Voice \global
                \new Voice \globalTempo
                \set Staff.autoBeaming = ##f % turns off all autobeaming

                \context Voice = "voice 1" {
                    % Segment: right hand, voice 1 (lié)
                    \override Voice.TextScript #'padding = #2.0
                    \override MultiMeasureRest #'expand-limit = 1

                    {   % Repeating stegment start here
                        % Segment: right hand, voice 1 (lié)
                        \set Score.repeatCommands = #'(start-repeat)
                        \clef "treble"
                        \key bes \major
                        \stemDown bes 2 ^\markup { \bold \large "Allegretto" } d' 4 ees' \stemNeutral  |
                        \stemDown c' 2 f' \stemNeutral  |
                        \stemDown d'' 8 [ c'' bes' a' ] g'' 2 \stemNeutral  |
                    } % close main repeat

                    {   % Alternative start here
                        % Segment: RH v1 alt1
                        \set Score.repeatCommands = #'((volta "1"))
                        \bar "|" 
                        \clef "treble"
                        \stemDown f' 2. g' 4 \stemNeutral  |
                    }

                    {   % Alternative start here
                        % Segment: RH v1 alt2
                        \set Score.repeatCommands = #'((volta #f) (volta "2") end-repeat)
                        \bar "|" 
                        \clef "treble"
                        \stemDown f' 2. g' 8 [ f' ] \stemNeutral  |
                        \stemDown bes 1 -\fermata \stemNeutral  |
                        \bar "|."
                        \set Score.repeatCommands = #'((volta #f))
                    }

                } % Voice

                \context Voice = "voice 4" {
                    % Segment: right hand, voice 2 (lié)
                    \override Voice.TextScript #'padding = #2.0
                    \override MultiMeasureRest #'expand-limit = 1

                    {   % Repeating stegment start here
                        % Segment: right hand, voice 2 (lié)
                        \set Score.repeatCommands = #'(start-repeat)
                        \clef "treble"
                        \key bes \major
                        \stemUp d'' 4 d'' g' 2 \stemNeutral  |
                        \stemUp c' 2 d'' 8 [ bes' ] r4 \stemNeutral  |
                        \stemUp g'' 8 [ a'' bes'' d''' ] g'' [ d'' ] ees'' r \stemNeutral  |
                    } % close main repeat

                    {   % Alternative start here
                        % Segment: RH v2 alt1
                        \set Score.repeatCommands = #'((volta "1"))
                        \bar "|" 
                        \clef "treble"
                        \stemUp ees'' 8 [ c'' ees'' f'' ] a'' [ bes'' a'' bes'' ] \stemNeutral  |
                    }

                    {   % Alternative start here
                        % Segment: RH v2 alt2
                        \set Score.repeatCommands = #'((volta #f) (volta "2") end-repeat)
                        \bar "|" 
                        \clef "treble"
                        \stemUp bes'' 8 [ g'' ] \stemDown f' 2. -\fermata _~ \stemNeutral  |
                        \stemUp f' 1 \stemNeutral  |
                        \bar "|."
                        \set Score.repeatCommands = #'((volta #f))
                    }

                } % Voice
            >> % Staff ends

            \context Staff = "track 2, Piano" << 
                \set Staff.instrumentName = \markup { \center-column { "Piano " } }
                \set Staff.midiInstrument = "Acoustic Grand Piano"
                \set Score.skipBars = ##t
                \set Staff.printKeyCancellation = ##f
                \new Voice \global
                \new Voice \globalTempo
                \set Staff.autoBeaming = ##f % turns off all autobeaming

                \context Voice = "voice 7" {
                    % Segment: left hand, voice 1 (lié)
                    \override Voice.TextScript #'padding = #2.0
                    \override MultiMeasureRest #'expand-limit = 1

                    {   % Repeating stegment start here
                        % Segment: left hand, voice 1 (lié)
                        \set Score.repeatCommands = #'(start-repeat)
                        \clef "bass"
                        \key bes \major
                        \stemUp bes, 2 bes, \stemNeutral  |
                        \stemUp a, 2 a, \stemNeutral  |
                        \stemUp c 2 c \stemNeutral  |
                    } % close main repeat

                    {   % Alternative start here
                        % Segment: LH v1 alt1
                        \set Score.repeatCommands = #'((volta "1"))
                        \bar "|" 
                        \clef "bass"
                        \stemUp < d g > 4 < d bes, > < g, ees, > 2 \stemNeutral  |
                    }

                    {   % Alternative start here
                        % Segment: LH v1 alt2
                        \set Score.repeatCommands = #'((volta #f) (volta "2") end-repeat)
                        \bar "|" 
                        \clef "bass"
                        s1  |
                        s1  |
                        \bar "|."
                        \set Score.repeatCommands = #'((volta #f))
                    }

                } % Voice

                \context Voice = "voice 10" {
                    % Segment: left hand, voice 2 (lié)
                    \override Voice.TextScript #'padding = #2.0
                    \override MultiMeasureRest #'expand-limit = 1

                    {   % Repeating stegment start here
                        % Segment: left hand, voice 2 (lié)
                        \set Score.repeatCommands = #'(start-repeat)
                        \clef "bass"
                        \key bes \major
                        \stemDown g,, 2 g, 8 [ f, ees, d, ] \stemNeutral  |
                        \stemDown g,, 4 a,, bes,, d, \stemNeutral  |
                        \stemDown bes, 4. bes, 8 a, 4. a, 8 \stemNeutral  |
                    } % close main repeat

                    {   % Alternative start here
                        % Segment: LH v2 alt1
                        \set Score.repeatCommands = #'((volta "1"))
                        \bar "|" 
                        \clef "bass"
                        \stemDown < g,, d, > 1 \stemNeutral  |
                    }

                    {   % Alternative start here
                        % Segment: LH v2 alt2
                        \set Score.repeatCommands = #'((volta #f) (volta "2") end-repeat)
                        \bar "|" 
                        \clef "bass"
                        \stemDown ees, 4 < a, ees, > < c g, > < ees bes, > \stemNeutral  |
                        \stemDown < c a, > 4 < bes, a, > < bes, bes,, > 2 -\fermata \stemNeutral  |
                        \bar "|."
                        \set Score.repeatCommands = #'((volta #f))
                    }

                } % Voice
            >> % Staff (final) ends
        >> % GrandStaff (final) 1

    >> % notes

    \layout {
        indent = 3.0\cm
        short-indent = 1.5\cm
        \context { \Staff \RemoveEmptyStaves }
        \context { \GrandStaff \accepts "Lyrics" }
    }
%     uncomment to enable generating midi file from the lilypond source
%         \midi {
%         } 
} % score
