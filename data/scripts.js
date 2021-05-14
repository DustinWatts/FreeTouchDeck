//Password toggle
function togglepassword() {
  var field = document.getElementById("password");
  if (field.type === "password") {
    field.type = "text";
  } else {
    field.type = "password";
  }
}

//Menu Handler
function openMenu(evt, menuitem) {
  // Declare all variables
  var i, tabcontent, tablinks;

  // Get all elements with class="tabcontent" and hide them
  tabcontent = document.getElementsByClassName("tabcontent");
  for (var i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  // Get all elements with class="tablinks" and remove the class "active"
  tablinks = document.getElementsByClassName("tablinks");
  for (var i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }

  // Show the current tab, and add an "active" class to the button that opened the tab
  document.getElementById(menuitem).style.display = "block";
  if (evt && evt.currentTarget) {
    evt.currentTarget.className += " active";
  }
}


//Load the array with all the dropdown menu items
var actionSelectItemData = [{
    text: 'Do Nothing',
    value: '0',
    subitems: [{
        text: '--',
        value: '0'
      }]
  },
  {
    text: 'Delay',
    value: '1',
    subitems: [{
        text: '100ms',
        value: '100'
      },
      {
        text: '200ms',
        value: '200'
      }
      ,
      {
        text: '500ms',
        value: '500'
      }
      ,
      {
        text: '1000ms',
        value: '1000'
      }
    ]
  },
  {
    text: 'Arrows and TAB',
    value: '2',
    subitems: [{
        text: '--',
        value: '0'
      },
      {
        text: 'UP Arrow',
        value: '1'
      },
      {
        text: 'DOWN Arrow',
        value: '2'
      },
      {
        text: 'LEFT Arrow',
        value: '3'
      },
      {
        text: 'RIGHT Arrow',
        value: '4'
      },
      {
        text: 'Backspace',
        value: '5'
      },
      {
        text: 'TAB',
        value: '6'
      },
      {
        text: 'Return',
        value: '7'
      },
      {
        text: 'Page Up',
        value: '8'
      },
      {
        text: 'Page Down',
        value: '9'
      },
      {
        text: 'Delete',
        value: '10'
      }
      ]
  },
  {
    text: 'Mediakey',
    value: '3',
    subitems: [{
        text: 'Mute',
        value: '1'
      },
      {
        text: 'Volume Down',
        value: '2'
      },
      {
        text: 'Volume Up',
        value: '3'
      },
      {
        text: 'Play/Pause',
        value: '4'
      },
      {
        text: 'Stop',
        value: '5'
      },
    {
      text: 'Next',
      value: '6'
    },
    {
      text: 'Previous',
      value: '7'
    }
    ]
  },
  {
    text: 'Letters',
    value: '4',
    subitems: [{
        text: '-space-',
        value: ' '
      },
      {
        text: 'a',
        value: 'a'
      },
      {
        text: 'b',
        value: 'b'
      },
      {
        text: 'c',
        value: 'c'
      },
      {
        text: 'd',
        value: 'd'
      },
      {
        text: 'e',
        value: 'e'
      },
      {
        text: 'f',
        value: 'f'
      },
      {
        text: 'g',
        value: 'g'
      },
      {
        text: 'h',
        value: 'h'
      },
      {
        text: 'i',
        value: 'i'
      },
      {
        text: 'j',
        value: 'j'
      },
      {
        text: 'k',
        value: 'k'
      },
      {
        text: 'l',
        value: 'l'
      },
      {
        text: 'm',
        value: 'm'
      },
      {
        text: 'n',
        value: 'n'
      },
      {
        text: 'o',
        value: 'o'
      },
      {
        text: 'p',
        value: 'p'
      },
      {
        text: 'q',
        value: 'q'
      },
      {
        text: 'r',
        value: 'r'
      },
      {
        text: 's',
        value: 's'
      },
      {
        text: 't',
        value: 't'
      },
      {
        text: 'u',
        value: 'u'
      },
      {
        text: 'v',
        value: 'v'
      },
      {
        text: 'w',
        value: 'w'
      },
      {
        text: 'x',
        value: 'x'
      },
      {
        text: 'y',
        value: 'y'
      },
      {
        text: 'z',
        value: 'z'
      }

      ]
  },
  {
    text: 'Option Keys',
    value: '5',
    subitems: [{
        text: 'Left CTRL',
        value: '1'
      },
      {
        text: 'Left Shift',
        value: '2'
      },
      {
        text: 'Left ALT',
        value: '3'
      },
      {
        text: 'Left GUI',
        value: '4'
      },
      {
        text: 'Right CTRL',
        value: '5'
      },
      {
        text: 'Right Shift',
        value: '6'
      },
      {
        text: 'Right ALT',
        value: '7'
      },
      {
        text: 'Right GUI',
        value: '8'
      },
      {
        text: 'Release All',
        value: '9'
      }]
  },
  {
    text: 'Function Keys',
    value: '6',
    subitems: [{
        text: 'F1',
        value: '1'
      },
      {
        text: 'F2',
        value: '2'
      },
      {
        text: 'F3',
        value: '3'
      },
      {
        text: 'F4',
        value: '4'
      },
      {
        text: 'F5',
        value: '5'
      },
      {
        text: 'F6',
        value: '6'
      },
      {
        text: 'F7',
        value: '7'
      },
      {
        text: 'F8',
        value: '8'
      },
      {
        text: 'F9',
        value: '9'
      },
      {
        text: 'F10',
        value: '10'
      },
      {
        text: 'F11',
        value: '11'
      },
      {
        text: 'F12',
        value: '12'
      },
      {
        text: 'F13',
        value: '13'
      },
      {
        text: 'F14',
        value: '14'
      },
      {
        text: 'F15',
        value: '15'
      },
      {
        text: 'F16',
        value: '16'
      },
      {
        text: 'F17',
        value: '17'
      },
      {
        text: 'F18',
        value: '18'
      },
      {
        text: 'F19',
        value: '19'
      },
      {
        text: 'F20',
        value: '20'
      },
      {
        text: 'F21',
        value: '21'
      },
      {
        text: 'F22',
        value: '22'
      },
      {
        text: 'F23',
        value: '23'
      },
      {
        text: 'F24',
        value: '24'
      }
      ]
  },
    {
    text: 'Numbers',
    value: '7',
    subitems: [{
        text: '0',
        value: '0'
      },
      {
        text: '1',
        value: '1'
      },
      {
        text: '2',
        value: '2'
      },
      {
        text: '3',
        value: '3'
      },
      {
        text: '4',
        value: '4'
      },
      {
        text: '5',
        value: '5'
      },
      {
        text: '6',
        value: '6'
      },
      {
        text: '7',
        value: '7'
      },
      {
        text: '8',
        value: '8'
      },
      {
        text: '9',
        value: '9'
      }

      ]
  },
    {
    text: 'Special Chars',
    value: '8',
    subitems: [{
        text: '.',
        value: '.'
      },
      {
        text: ',',
        value: ','
      },
      {
        text: '!',
        value: '!'
      },
      {
        text: '?',
        value: '?'
      },
      {
        text: '/',
        value: '/'
      },
      {
        text: '+',
        value: '+'
      },
      {
        text: '-',
        value: '-'
      },
      {
        text: '&',
        value: '&'
      },
      {
        text: '^',
        value: '^'
      },
      {
        text: '%',
        value: '%'
      },
      {
        text: '*',
        value: '*'
      },
      {
        text: '#',
        value: '#'
      },
      {
        text: '$',
        value: '$'
      }

      ]
  },
    {
    text: 'Combos',
    value: '9',
    subitems: [{
        text: 'LEFT CTRL+SHIFT',
        value: '1'
      },
      {
        text: 'LEFT ALT+SHIFT',
        value: '2'
      },
      {
        text: 'LEFT GUI+SHIFT',
        value: '3'
      },
      {
        text: 'LEFT CTRL+GUI',
        value: '4'
      },
      {
        text: 'LEFT ALT+GUI',
        value: '5'
      },
      {
        text: 'LEFT CTRL+ALT',
        value: '6'
      },
      {
        text: 'LEFT CTRL+ALT+GUI',
        value: '7'
      },
      {
        text: 'RIGHT CTRL+SHIFT',
        value: '8'
      },
      {
        text: 'RIGHT ALT+SHIFT',
        value: '9'
      },
      {
        text: 'RIGHT GUI+SHIFT',
        value: '10'
      },
      {
        text: 'RIGHT CTRL+GUI',
        value: '11'
      },
      {
        text: 'RIGHT ALT+GUI',
        value: '12'
      },
      {
        text: 'RIGHT CTRL+ALT',
        value: '13'
      },
      {
        text: 'RIGHT CTRL+ALT+GUI',
        value: '14'
      }

      ]
  },
    {
    text: 'Helpers',
    value: '10',
    subitems: [{
        text: 'Helper 1',
        value: '1'
      },
      {
        text: 'Helper 2',
        value: '2'
      },
      {
        text: 'Helper 3',
        value: '3'
      },
      {
        text: 'Helper 4',
        value: '4'
      },
      {
        text: 'Helper 5',
        value: '5'
      },
      {
        text: 'Helper 6',
        value: '6'
      },
      {
        text: 'Helper 7',
        value: '7'
      },
      {
        text: 'Helper 8',
        value: '8'
      },
      {
        text: 'Helper 9',
        value: '9'
      },
      {
        text: 'Helper 10',
        value: '10'
      },
      {
        text: 'Helper 11',
        value: '11'
      }

      ]
  }
];

function populateActionSelects($el, i) {
  $.each(actionSelectItemData, function () {
    for (var a = 0; a < 3; a++) {
      var $actionSelect = $('[name="screenButton' + i + 'Action' + a + '"]', $el);
      $actionSelect.append($('<option>', {value:this.value, text:this.text}));
    }

    for (var a = 0; a < 3; a++) {
      (function($, i, a, $el){
        $('[name="screenButton' + i + 'Action' + a + '"]', $el).change(function () {
          var action = $(this).val();
          var $valueSelect = $('[name="screenButton' + i + 'Value' + a + '"]', $el);

          $valueSelect.empty();
          $.each(actionSelectItemData[action].subitems, function () {
            $valueSelect.append($('<option>', {value:this.value, text:this.text}));
          });
        }).change();
      })(jQuery, i, a, $el);
    }
  });
}


function createCheck(filename) {
    return `<input form="delete" type="checkbox" id="${filename}" name="${filename}" value="${filename}"><label for="${filename}"> ${filename}</label><br>`;
}

function fillvaluelist(targetNode, srcArray) {

  targetNode.innerHTML = srcArray.reduce((options, { value, text }) =>
    options += `<option value="${value}">${text}</option>`,
    '');

}

/**
Fetch the menu config from a given url, and populate the relevant forms

@param url The url to fetch from
@param $el The jquery dom node/container to scope the population of data
*/
function getMenuConfig(url, $el){
  // Load Menu
  fetch(url)
  .then((response) => {
    return response.json()
  })
  .then((data) => {

      console.log('menu data returned', url, $el);
      console.log('data', data);

      for (var i = 0; i < 5; i++) {
        var $screenLogo = $('[name="screenLogo' + i + '"]', $el);
        var screenLogoData = data['logo' + i];
        $screenLogo.append(new Option(screenLogoData));

        if(data['button' + i].latch) {
          var $latchButton = $('[name="screenButton' + i + 'Latch"]', $el);
          $latchButton.prop('checked', true);
        }

        var $latchLogo = $('[name="screenLatchLogo' + i + '"]', $el);
        if(data['button' + i].latchlogo == "") {
          $latchLogo.append(new Option("---"));
        } else {
          $latchLogo.append(data['button' + i].latchlogo);
          $latchLogo.append(new Option("---"));
        }

        for (var a = 0; a < 3; a++) {
          var $actionSelect = $('[name="screenButton' + i + 'Action' + a + '"]', $el);
          $actionSelect.val(data['button' + i].actionarray[a]);
          $actionSelect.change();

          var $valueSelect = $('[name="screenButton' + i + 'Value' + a + '"]', $el);
//          fillvaluelist($valueSelect, actionSelectItemData[data['button' + i].actionarray[a]].subitems);
          $valueSelect.val(data['button' + i].valuearray[a]);
        }
      }

      $('[name="screenLogo5"]', $el).append(new Option("home.bmp"));
    })
    .catch((err) => {
    console.log(err)
    })
}

function getlogoconfig(){

  var logolist = []; // Array holding the logos (empty to start)

  var selects = document.getElementsByClassName('images');

  // Loading all the logo's
  fetch('config/homescreen.json')
    .then((response) => {
    return response.json()
  })
  .then((data) => {

    document.getElementById("homescreenlogo0").add(new Option(data.logo0));
    document.getElementById("homescreenlogo1").add(new Option(data.logo1));
    document.getElementById("homescreenlogo2").add(new Option(data.logo2));
    document.getElementById("homescreenlogo3").add(new Option(data.logo3));
    document.getElementById("homescreenlogo4").add(new Option(data.logo4));
    document.getElementById("homescreenlogo5").add(new Option(data.logo5));

  })
  .then((data) => {
    fetch('/list?dir=/logos')
      .then((response3) => {
        return response3.json()
      })
      .then((data3) => {
        var checkboxlist = '<form method="post" id="delete" action="/editor">';
        data3.forEach(obj => {
          Object.entries(obj).forEach(([key, value]) => {
            
            var input = createCheck(value); // Creates the html for the checkbox
                checkboxlist += input;

          });

        });
        checkboxlist += '<br><button style="cursor: pointer;"" type="save">Delete files</button></form>';
        document.getElementById("deletefilelist").innerHTML = checkboxlist;

      })
      .catch((err) => {
        console.log(err)
      })

  })
  .then((data) => {
    fetch('/info')
      .then((response4) => {
        return response4.json()
      })
      .then((data4) => {
        var divcontent = '';
        data4.forEach(obj => {
          Object.entries(obj).forEach(([key, value]) => {
            
            divcontent += `${key}: ${value}<br>`;
          });

        });
        document.getElementById("infocontent").innerHTML = divcontent;

      })
      .catch((err) => {
        console.log(err)
      })

  })

  .then((data) => {
    fetch('/list?dir=/logos')
      .then((response2) => {
        return response2.json()
      })
      .then((data2) => {
        data2.forEach(obj => {
          Object.entries(obj).forEach(([key, value]) => {
            logolist.push(`${value}`);
            var i = selects.length;
            while (i--) {
              var select = selects[i];
              select.add(new Option(`${value}`, `${value}`));
            }

          });
        });

      })
      .then(() => {
        document.getElementById("contentloading").style.display = "none";
        document.getElementById('ball-loader').style.display = "none";
        document.getElementById("maincontent").style.display = "block";
      })
      .catch((err) => {
        console.log(err)
      })

  })
  .catch((err) => {
    console.log(err)
  })


}

// Function to get Sleep settings:
function getwificonfig(){

    // Load Menu 5
fetch('config/wificonfig.json')
  .then((response) => {
    return response.json()
  })
  .then((data) => {


  document.getElementById("ssid").value = data.ssid;
  document.getElementById("password").value = data.password;
  document.getElementById("wifimode").value = data.wifimode;
  document.getElementById("wifihostname").value = data.wifihostname;
  document.getElementById("attempts").value = data.attempts;
  document.getElementById("attemptdelay").value = data.attemptdelay;
  

  })
  .catch((err) => {
    console.log(err)
  })
}

$(document).ready(function() {
  openMenu(event, 'intro');

  for (var i = 0; i < 5; i++) {
    var $el = $('#savemenu' + (i+1));
    for (var buttonId = 0; buttonId < 5; buttonId++) {
      populateActionSelects($el, buttonId);
    }
  }

  // Load Colors and then other configs
  fetch('config/general.json')
    .then((response) => {
      return response.json()
    })
    .then((data) => {

      document.getElementById("menubuttoncolor").value = data.menubuttoncolor;
      document.getElementById("functionbuttoncolor").value = data.functionbuttoncolor;
      document.getElementById("latchcolor").value = data.latchcolor;
      document.getElementById("background").value = data.background;
      document.getElementById("sleepenable").value = data.sleepenable;
      document.getElementById("sleeptimer").value = data.sleeptimer;
      document.getElementById("beep").value = data.beep;
      document.getElementById("modifier1").value = data.modifier1;
      document.getElementById("modifier2").value = data.modifier2;
      document.getElementById("modifier3").value = data.modifier3;
      document.getElementById("helperdelay").value = data.helperdelay;

    }).then(() => {
      getwificonfig();
    })
    .then(() => {
      getMenuConfig('config/menu1.json', $('#savemenu1'));
    })
    .then(() => {
      getMenuConfig('config/menu2.json', $('#savemenu2'));
    })
    .then(() => {
      getMenuConfig('config/menu3.json', $('#savemenu3'));
    })
    .then(() => {
      getMenuConfig('config/menu4.json', $('#savemenu4'));
    })
    .then(() => {
      getMenuConfig('config/menu5.json', $('#savemenu5'));
    })
    .then(() => {
      getlogoconfig();
    })
    .catch((err) => {
      console.log(err)
    });
});

