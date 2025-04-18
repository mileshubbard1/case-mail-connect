
export interface CalendarEvent {
  id: string;
  title: string;
  date: Date;
  type: 'court' | 'client' | 'internal';
  case?: string;
  associatedEmails: AssociatedEmail[];
}

export interface AssociatedEmail {
  id: string;
  subject: string;
  sender: string;
  date: string;
  preview: string;
}
