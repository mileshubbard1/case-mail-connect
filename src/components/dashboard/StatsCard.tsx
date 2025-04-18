
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Mail, FileText, FileCheck, Clock } from "lucide-react";

interface StatProps {
  title: string;
  value: string;
  description: string;
  icon: React.ReactNode;
  trend?: {
    value: number;
    label: string;
  };
}

const Stat = ({ title, value, description, icon, trend }: StatProps) => (
  <Card>
    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
      <CardTitle className="text-sm font-medium">{title}</CardTitle>
      <div className="h-5 w-5 text-legal-blue">{icon}</div>
    </CardHeader>
    <CardContent>
      <div className="text-2xl font-bold">{value}</div>
      <p className="text-xs text-muted-foreground">{description}</p>
      {trend && (
        <div className={`text-xs mt-1 ${trend.value >= 0 ? 'text-green-600' : 'text-red-600'}`}>
          {trend.value >= 0 ? '+' : ''}{trend.value}% {trend.label}
        </div>
      )}
    </CardContent>
  </Card>
);

export function StatsCard() {
  return (
    <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
      <Stat 
        title="Unread Emails" 
        value="24" 
        description="Across all cases"
        icon={<Mail />}
        trend={{ value: 12, label: "from last week" }}
      />
      <Stat 
        title="Active Cases" 
        value="16" 
        description="Currently in progress"
        icon={<FileText />}
      />
      <Stat 
        title="Documents" 
        value="132" 
        description="Total documents"
        icon={<FileCheck />}
        trend={{ value: 8, label: "this month" }}
      />
      <Stat 
        title="Upcoming Deadlines" 
        value="7" 
        description="Within next 14 days"
        icon={<Clock />}
      />
    </div>
  );
}
