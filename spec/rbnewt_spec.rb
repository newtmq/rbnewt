require 'spec_helper'

describe Newt do
  before :all do
    @session  = Newt::STOMP.new
  end

  it 'could open stomp session' do
    expect(@session).not_to be nil
  end
  it 'cloud close stomp session' do
    expect(@session.close).not_to be nil
  end
end
