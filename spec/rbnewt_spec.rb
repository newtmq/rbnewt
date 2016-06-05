require 'spec_helper'

describe Newt do
  QNAME = '/queue/_rspec_qname'

  class CBCheck
    attr_reader :connected_cb_is_called, :message_cb_is_called, :error_cb_is_called
    def initialize
      @connected_cb_is_called = false
      @message_cb_is_called = false
      @error_cb_is_called = false
    end

    def cb_connected(headers, body)
      @connected_cb_is_called = true
    end
    def cb_message(headers, body)
      @message_cb_is_called = true
      false
    end
    def cb_error(headers, body)
      @error_cb_is_called = true
      false
    end
  end

  context "case of success" do
    let(:qname) { '/queue/_rpsec_qname' }
    before :all do
      @callback = CBCheck.new
      @session  = Newt::STOMP.new(@callback)
    end

    it "success to open session" do
      expect(@session).not_to be_nil
    end
    it "success to send message" do
      expect(@session.publish(qname, 'message')).not_to be_nil
    end
    it "success to receive message" do
      @session.subscribe(qname)
      expect(@session.run).not_to be_nil
    end
    it "success to be invoked callback" do
      expect(@callback.connected_cb_is_called).to be true
      expect(@callback.message_cb_is_called).to be true
      expect(@callback.error_cb_is_called).to be false
    end
  end
  context "case of failed to authenticate user" do
    before :all do
      @callback = CBCheck.new
      @session  = Newt::STOMP.new(@callback, :userid => 'invalid_userid', :passwd => 'invalid_passwd')
    end

    it "failed to open session" do
      @session.run
      expect(@callback.connected_cb_is_called).to be false
      expect(@callback.message_cb_is_called).to be false
      expect(@callback.error_cb_is_called).to be true
    end
  end
end
