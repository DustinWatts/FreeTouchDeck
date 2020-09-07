var items = selecteditems.slice(0); // Create a duplicate of the selecteditems array for now

$(function () {
  var temp = {};
  // Menu1 Button0
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button0action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button0action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button0value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button0action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button0action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button0value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button0action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button0action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button0value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1Button 1
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button1action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button1action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button1value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button1action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button1action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button1value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button1action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button1action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button1value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 Button2
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button2action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button2action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button2value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button2action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button2action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button2value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button2action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button2action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button2value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button3
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button3action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button3action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button3value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button3action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button3action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button3value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button3action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button3action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button3value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button4
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button4action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button4action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button4value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button4action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button4action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button4value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button4action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button4action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button4value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button5
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button5action0');
    temp[this.value] = this.subitems;
  });

  $('#screen1button5action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button5value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button5action1');
    temp[this.value] = this.subitems;
  });

  $('#screen1button5action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button5value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen1button5action2');
    temp[this.value] = this.subitems;
  });

  $('#screen1button5action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen1button5value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 Button0
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button0action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button0action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button0value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button0action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button0action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button0value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button0action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button0action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button0value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1Button 1
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button1action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button1action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button1value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button1action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button1action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button1value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button1action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button1action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button1value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 Button2
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button2action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button2action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button2value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button2action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button2action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button2value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button2action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button2action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button2value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button3
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button3action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button3action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button3value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button3action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button3action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button3value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button3action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button3action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button3value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button4
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button4action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button4action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button4value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button4action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button4action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button4value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button4action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button4action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button4value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // Menu1 button5
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button5action0');
    temp[this.value] = this.subitems;
  });

  $('#screen2button5action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button5value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button5action1');
    temp[this.value] = this.subitems;
  });

  $('#screen2button5action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button5value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen2button5action2');
    temp[this.value] = this.subitems;
  });

  $('#screen2button5action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen2button5value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3 Button0
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button0action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button0action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button0value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button0action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button0action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button0value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button0action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button0action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button0value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3Button 1
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button1action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button1action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button1value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button1action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button1action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button1value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button1action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button1action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button1value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3 Button2
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button2action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button2action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button2value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button2action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button2action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button2value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button2action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button2action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button2value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3 button3
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button3action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button3action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button3value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button3action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button3action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button3value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button3action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button3action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button3value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3 button4
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button4action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button4action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button4value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button4action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button4action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button4value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button4action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button4action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button4value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu3 button5
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button5action0');
    temp[this.value] = this.subitems;
  });

  $('#screen3button5action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button5value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button5action1');
    temp[this.value] = this.subitems;
  });

  $('#screen3button5action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button5value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen3button5action2');
    temp[this.value] = this.subitems;
  });

  $('#screen3button5action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen3button5value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4 Button0
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button0action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button0action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button0value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button0action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button0action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button0value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button0action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button0action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button0value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4Button 1
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button1action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button1action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button1value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button1action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button1action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button1value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button1action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button1action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button1value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4 Button2
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button2action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button2action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button2value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button2action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button2action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button2value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button2action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button2action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button2value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4 button3
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button3action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button3action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button3value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button3action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button3action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button3value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button3action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button3action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button3value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4 button4
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button4action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button4action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button4value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button4action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button4action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button4value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button4action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button4action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button4value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu4 button5
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button5action0');
    temp[this.value] = this.subitems;
  });

  $('#screen4button5action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button5value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button5action1');
    temp[this.value] = this.subitems;
  });

  $('#screen4button5action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button5value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen4button5action2');
    temp[this.value] = this.subitems;
  });

  $('#screen4button5action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen4button5value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5 Button0
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button0action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button0action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button0value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button0action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button0action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button0value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button0action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button0action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button0value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5Button 1
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button1action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button1action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button1value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button1action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button1action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button1value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button1action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button1action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button1value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5 Button2
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button2action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button2action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button2value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button2action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button2action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button2value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button2action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button2action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button2value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5 button3
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button3action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button3action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button3value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button3action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button3action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button3value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button3action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button3action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button3value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5 button4
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button4action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button4action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button4value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button4action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button4action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button4value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button4action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button4action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button4value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  // menu5 button5
  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button5action0');
    temp[this.value] = this.subitems;
  });

  $('#screen5button5action0')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button5value0');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button5action1');
    temp[this.value] = this.subitems;
  });

  $('#screen5button5action1')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button5value1');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();

  $.each(items, function () {
    $('<option />')
      .attr('value', this.value)
      .html(this.name)
      .appendTo('#screen5button5action2');
    temp[this.value] = this.subitems;
  });

  $('#screen5button5action2')
    .change(function () {
      var value = $(this).val();
      var menu = $('#screen5button5value2');

      menu.empty();
      $.each(temp[value], function () {
        $('<option />')
          .attr('value', this.value)
          .html(this.name)
          .appendTo(menu);
      });
    })
    .change();
});
