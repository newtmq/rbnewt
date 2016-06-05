# rbnewt
[![Build Status](https://travis-ci.org/newtmq/rbnewt.svg?branch=master)](https://travis-ci.org/newtmq/rbnewt)

This is a yet another Ruby binding of STOMP client. This aims to make an effect for your applicaiton using STOMP to be faster.

## Installation

### install 'libstomp'
`rbnewt` depends [libstomp](https://github.com/userlocalhost2000/libstomp) which is an implementation of STOMP client written in C. You can install according to the following procedure.

#### download source and expand it
```Bash
$ https://github.com/userlocalhost2000/libstomp/archive/master.zip
$ unzip master.zip
```

#### installing libstomp
To build libstomp from source, you may install `autoconf`.
```Bash
$ cd libstomp-master
$ autoreconf -i
$ ./configure
$ make
$ sudo make install
```

### install 'rbnewt'

Add this line to your application's Gemfile:

```ruby
gem 'rbnewt'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install rbnewt

## Getting Started
Here is an example to publish and subscribe message.
```Ruby
require 'rbnewt'

class MyCallback
  # This method is called when rbnewt received CONNECTED frame that is sent when session
  # authentication is successed.
  def cb_connected(headers, body)
    puts "Connected ..."
  end

  # This method is called when rbnewt received MESSAGE frames that ware in specified queue
  # by `subscribe` method of rbnewt.
  def cb_message(headers, body)
    puts "Received some message"

    # When you return 'false' in callback method, receving processing is stopped
    # and the session with STOMP server will be closed.
    false
  end
  # This method is called when rbnewt received ERROR frames.
  def cb_error(headers, body)
    # some error handing
    false
  end
end

# making original callback object which handles messages that is sent by STOMP server.
cb_obj = MyCallback.new

# open a session with stomp server
# args:
#   - first  : callback object
#   - second : information about STOMP server to connects. You can specify followings.
#     {
#       :server => 'hostname of STOMP server'           (default: 'localhost')
#       :port   => 'port number to connect'             (default: '61613')
#       :userid => 'userid to authenticate with server' (default: 'guest')
#       :passwd => 'passwd to authenticate with server' (default: 'guest')
#     }
session = Newt::STOMP.new(cb_obj)

# publishing message to the queue which is specified in first argument
session.publish('/queue/name', 'message body')

# sending SUBSCRIBE frame to receive message of '/queue/name' queue.
session.subscribe('/queue/name')

# blocking to receive frames which are sent by server.
session.run
```

## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).
