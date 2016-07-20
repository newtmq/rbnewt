require 'spec_helper'

describe Newt do
  context "case of success" do
    let(:qname) { '/queue/_rpsec_qname' }
    let(:msg_context) { 'this is the context of sending message' }
    before :all do
      @session  = Newt::STOMP.new()
    end

    it "success to open session" do
      expect(@session).not_to be_nil
    end
    it "success to send message" do
      expect(@session.publish(qname, msg_context)).not_to be_nil
    end
    it "success to send SUBSCRIBE message" do
      expect(@session.subscribe(qname)).to be true
    end
    it "success to recv message" do
      message = @session.receive

      expect(message).not_to be_nil
      expect(message.command).to match 'MESSAGE'
      expect(message.headers).not_to be_nil
      expect(message.body).not_to be_nil
      expect(message.body).to match msg_context
    end
  end
end
