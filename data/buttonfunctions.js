	var items = [{
		    name: 'Do Nothing',
		    value: '0',
		    subitems: [{
		        name: '--',
		        value: '0'
		      }]
		  },
		  {
		    name: 'Delay',
		    value: '1',
		    subitems: [{
		        name: '100ms',
		        value: '100'
		      },
		      {
		        name: '200ms',
		        value: '200'
		      }
		      ,
		      {
		        name: '500ms',
		        value: '500'
		      }
		      ,
		      {
		        name: '1000ms',
		        value: '1000'
		      }
		    ]
		  },
		  {
		    name: 'Extra Options',
		    value: '2',
		    subitems: [{
		        name: '--',
		        value: '0'
		      }]
		  },
		  {
		    name: 'Mediakey',
		    value: '3',
		    subitems: [{
		        name: 'Mute',
		        value: '1'
		      },
		      {
		        name: 'Volume Down',
		        value: '2'
		      },
		      {
		        name: 'Volume Up',
		        value: '3'
		      },
		      {
		        name: 'Play/Pause',
		        value: '4'
		      },
		      {
		        name: 'Stop',
		        value: '5'
		      }
		    ]
		  },
		  {
		    name: 'Send Letter',
		    value: '4',
		    subitems: [{
		        name: 'a',
		        value: 'a'
		      },
		      {
		        name: 'b',
		        value: 'b'
		      },
		      {
		        name: 'c',
		        value: 'c'
		      },
		      {
		        name: 'd',
		        value: 'd'
		      },
		      {
		        name: 'e',
		        value: 'e'
		      },
		      {
		        name: 'f',
		        value: 'f'
		      },
		      {
		        name: 'g',
		        value: 'g'
		      },
		      {
		        name: 'h',
		        value: 'h'
		      },
		      {
		        name: 'i',
		        value: 'i'
		      },
		      {
		        name: 'j',
		        value: 'j'
		      },
		      {
		        name: 'k',
		        value: 'k'
		      },
		      {
		        name: 'l',
		        value: 'l'
		      },
		      {
		        name: 'm',
		        value: 'm'
		      },
		      {
		        name: 'n',
		        value: 'n'
		      },
		      {
		        name: 'o',
		        value: 'o'
		      },
		      {
		        name: 'p',
		        value: 'p'
		      },
		      {
		        name: 'q',
		        value: 'q'
		      },
		      {
		        name: 'r',
		        value: 'r'
		      },
		      {
		        name: 's',
		        value: 's'
		      },
		      {
		        name: 't',
		        value: 't'
		      },
		      {
		        name: 'u',
		        value: 'u'
		      },
		      {
		        name: 'v',
		        value: 'v'
		      },
		      {
		        name: 'w',
		        value: 'w'
		      },
		      {
		        name: 'x',
		        value: 'x'
		      },
		      {
		        name: 'y',
		        value: 'y'
		      },
		      {
		        name: 'z',
		        value: 'z'
		      }

		      ]
		  },
		  {
		    name: 'Function Keys',
		    value: '5',
		    subitems: [{
		        name: 'Left CTRL',
		        value: '1'
		      },
		      {
		        name: 'Left Shift',
		        value: '2'
		      },
		      {
		        name: 'Left ALT',
		        value: '3'
		      },
		      {
		        name: 'Left GUI',
		        value: '4'
		      },
		      {
		        name: 'Right CTRL',
		        value: '5'
		      },
		      {
		        name: 'Right Shift',
		        value: '6'
		      },
		      {
		        name: 'Right ALT',
		        value: '7'
		      },
		      {
		        name: 'Right GUI',
		        value: '8'
		      },
		      {
		        name: 'Release All',
		        value: '9'
		      }]
		  }
		];

		$(function() {
		  var temp = {};
		  // Menu1 Button0
		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button0action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button0action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button0value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button0action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button0action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button0value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button0action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button0action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button0value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1Button 1
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button1action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button1action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button1value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button1action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button1action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button1value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button1action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button1action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button1value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 Button2
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button2action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button2action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button2value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button2action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button2action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button2value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button2action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button2action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button2value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button3
		$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button3action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button3action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button3value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button3action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button3action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button3value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button3action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button3action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button3value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button4
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button4action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button4action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button4value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button4action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button4action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button4value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button4action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button4action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button4value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button5
$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button5action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button5action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button5value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button5action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button5action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button5value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen1button5action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen1button5action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen1button5value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 Button0
		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button0action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button0action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button0value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button0action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button0action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button0value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button0action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button0action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button0value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1Button 1
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button1action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button1action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button1value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button1action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button1action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button1value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button1action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button1action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button1value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 Button2
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button2action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button2action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button2value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button2action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button2action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button2value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button2action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button2action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button2value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button3
		$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button3action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button3action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button3value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button3action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button3action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button3value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button3action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button3action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button3value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button4
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button4action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button4action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button4value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button4action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button4action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button4value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button4action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button4action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button4value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // Menu1 button5
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button5action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button5action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button5value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button5action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button5action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button5value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen2button5action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen2button5action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen2button5value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3 Button0
		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button0action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button0action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button0value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button0action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button0action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button0value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button0action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button0action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button0value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3Button 1
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button1action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button1action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button1value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button1action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button1action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button1value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button1action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button1action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button1value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3 Button2
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button2action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button2action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button2value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button2action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button2action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button2value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button2action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button2action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button2value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3 button3
		$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button3action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button3action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button3value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button3action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button3action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button3value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button3action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button3action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button3value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3 button4
			$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button4action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button4action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button4value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button4action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button4action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button4value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button4action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button4action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button4value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu3 button5
		$.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button5action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button5action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button5value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button5action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button5action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button5value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen3button5action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen3button5action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen3button5value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  // menu4 Button0
		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button0action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button0action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button0value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button0action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button0action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button0value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button0action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button0action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button0value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button1action0");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button1action0").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button1value0");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button1action1");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button1action1").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button1value1");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();

		  $.each(items, function() {
		    $("<option />")
		      .attr("value", this.value)
		      .html(this.name)
		      .appendTo("#screen4button1action2");
		    temp[this.value] = this.subitems;
		  });

		  $("#screen4button1action2").change(function() {
		    var value = $(this).val();
		    var menu = $("#screen4button1value2");

		    menu.empty();
		    $.each(temp[value], function() {
		      $("<option />")
		        .attr("value", this.value)
		        .html(this.name)
		        .appendTo(menu);
		    });
		  }).change();


		});