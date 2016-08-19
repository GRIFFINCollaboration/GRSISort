# author: stefaan.himpe@gmail.com
# license: MIT
from Tkinter import Text, END
import re

class AnsiColorText(Text):
      """
  class to convert text with ansi color codes to
  text with tkinter color tags

  for now we ignore all but the simplest color directives
  see http://www.termsys.demon.co.uk/vtansi.htm for a list of
  other directives

  it has not been thoroughly tested, but it works well enough for demonstration purposes
  """
      foreground_colors = {
              'bright' : {
                              '30' : 'Black',
                              '31' : 'Red',
                              '32' : 'Green',
                              '33' : 'Brown',
                              '34' : 'Blue',
                              '35' : 'Purple',
                              '36' : 'Cyan',
                              '37' : 'White'
                              },
              'dim'    :  {
                              '30' : 'DarkGray',
                              '31' : 'LightRed',
                              '32' : 'LightGreen',
                              '33' : 'Yellow',
                              '34' : 'LightBlue',
                              '35' : 'Magenta',
                              '36' : 'Pink',
                              '37' : 'White'
                              }
          }

      background_colors= {
              'bright' : {
                              '40' : 'Black',
                              '41' : 'Red',
                              '42' : 'Green',
                              '43' : 'Brown',
                              '44' : 'Blue',
                              '45' : 'Purple',
                              '46' : 'Cyan',
                              '47' : 'White'
                              },
              'dim'    :  {
                              '40' : 'DarkGray',
                              '41' : 'LightRed',
                              '42' : 'LightGreen',
                              '43' : 'Yellow',
                              '44' : 'LightBlue',
                              '45' : 'Magenta',
                              '46' : 'Pink',
                              '47' : 'White'
                              }
          }

      # define some regexes which will come in handy in filtering
      # out the ansi color codes
      color_pat = re.compile('\x01?\x1b\[([\d+;]*?)m\x02?')
      inner_color_pat = re.compile("^(\d+;?)+$")

      def __init__(self, *args, **kwargs):
        """
    initialize our specialized tkinter Text widget
    """
        Text.__init__(self, *args, **kwargs)
        self.known_tags = set([])
        # register a default color tag
        self.register_tag("30", "White", "Black")
        self.reset_to_default_attribs()

      def reset_to_default_attribs(self):
        self.tag = '30'
        self.bright = 'bright'
        self.foregroundcolor = 'White'
        self.backgroundcolor = 'Black'

      def register_tag(self, txt, foreground, background):
        """
    register a tag with name txt and with given
    foreground and background color
    """
        self.tag_config(txt, foreground=foreground, background=background)
        self.known_tags.add(txt)

      def write(self, text, is_editable=False):
        """
    add text to the text widget
    """
        # first split the text at color codes, stripping stuff like the <ESC>
        # and \[ characters and keeping only the inner "0;23"-like codes
        segments = AnsiColorText.color_pat.split(text)
        if segments:
          for text in segments:
            # a segment can be regular text, or it can be a color pattern
            if AnsiColorText.inner_color_pat.match(text):
              # if it's a color pattern, check if we already have
              # registered a tag for it
              if text not in self.known_tags:
                # if tag not yet registered,
                # extract the foreground and background color
                # and ignore the other things
                parts = text.split(";")
                for part in parts:
                  if part in AnsiColorText.foreground_colors[self.bright]:
                    self.foregroundcolor = AnsiColorText.foreground_colors[self.bright][part]
                  elif part in AnsiColorText.background_colors[self.bright]:
                    self.backgroundcolor = AnsiColorText.background_colors[self.bright][part]
                  else:
                    for ch in part:
                      if ch == '0' :
                        # reset all attributes
                        self.reset_to_default_attribs()
                      if ch == '1' :
                        # define bright colors
                        self.bright = 'bright'
                      if ch == '2' :
                        # define dim colors
                        self.bright = 'dim'

                self.register_tag(text,
                                  foreground=self.foregroundcolor,
                                  background=self.backgroundcolor)
              # remember that we switched to this tag
              self.tag = text
            elif text == '':
              # reset tag to black
              self.tag = '30' # black
            else:
              # no color pattern, insert text with the currently selected
              # tag
              self.insert(END,text,self.tag)
